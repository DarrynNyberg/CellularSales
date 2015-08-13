<?php
// $Id$

/**
 * @file
 * Administration page callbacks for the store locator module.
 */


/**
 * Create a new store node from CSOKI import.
 */
function cs_import_store_node($data, $install = false) {
  $node = new stdClass();
  $node->type = 'store';
  node_object_prepare($node);
  $node->title = "{$data['market']}-{$data['loc']}"; // Store code
  $node->language = LANGUAGE_NONE;
  $node->locations[0]['name'] = $data['name'];
  $node->locations[0]['street'] = $data['street'];
  $node->locations[0]['additional'] = $data['addr2'];
  $node->locations[0]['city'] = $data['city'];
  $node->locations[0]['province'] = $data['state'];
  $node->locations[0]['postal_code'] = $data['zip'];
  $node->locations[0]['country'] = "us";
  $node->locations[0]['accuracy'] = $install ? -1 : 0; // Use this to prevent presave from geocoding every one and using up all geo queries. Cron will update slowly.
  // Simulate the empty user input fields "locpick".
  $node->locations[0]['locpick']['user_latitude'] = NULL;
  $node->locations[0]['locpick']['user_longitude'] = NULL;
  $node->locations[0]['locpick']['source'] = 3; // Default to 3 (google) we will look everything up.
  $node->locations[0]['is_primary'] = 0;
  $node->locations[0]['csoki']['allow'] = 'allow';
  $node->locations[0]['outlet'] = $data['outlet'];
  $node->locations[0]['fax'] = $data['fax'];
  $node->locations[0]['phone'] = $data['t_phone'] =='' ? $data['phone'] : $data['t_phone'];
  $node->locations[0]['hours'] = $data['hours'];
  $node = node_submit($node);
  node_save($node);
  return $node->nid;
}


/**
 * Parse store hours into something readable and compact.
 * Format provided in the data stream is:
 * Monday/08:00-20:00|Tuesday/08:00-20:00|Wednesday/08:00-20:00|Thursday/08:00-20:00|Friday/08:00-20:00|Saturday/08:00-20:00|Sunday/10:00-19:00 
 */
function cs_parse_store_hours($data) {
  $store_hours = '';
  $data = str_replace(array("\r", "\n"), '', $data); // Fix returns in the middle of the data.
  $specs = explode('|', trim($data));
  if (count($specs) == 7) {
    $days = array();
    $hours = array();
    foreach ($specs as $spec) {
      $parts = explode('/', $spec);
      $days[] = substr($parts[0], 0, 3);
      if (!isset($parts[1])) {
        continue;
      }
      $times = explode('-', $parts[1]);
      foreach ($times as $i => $time) {
        $start = (int)substr($time, 0, 2);
        if ($start < 12) {
          $times[$i] = $start . substr($time, 2) . 'am';
        } else
          $times[$i] = ($start == 12 ? 12 : $start - 12) . substr($time, 2) . 'pm';
      }
      $hours[] = $times[0] . '-' . $times[1];
    }

    $last = 0;
    $store_hours = $days[0];
    foreach ($hours as $i => $time) {
      if ($time != $hours[$last]) {
        if ($i - $last > 1) {
          $store_hours .= '-' . $days[$i-1];
        }
        $store_hours .= ' ' . $hours[$last];
        $store_hours .= '<br />';
        $store_hours .= $days[$i];
        $last = $i;
      }
    }
    if ($last == 0) { // All the same
      $store_hours .= '-' . $days[6];
    }
    $store_hours .= ' ' . $hours[$last];
  }
  return $store_hours;
}
 


/**
 * Contact CSOKI and sync store data with the database. 
 */
function cs_csoki_store_import($import_path = null, $install = false) {
  $wd = __FUNCTION__;
  if ($import_path) {
    $storeList = file_get_contents($import_path);
  } else {
    // Load the data - All location data in one call! Market setting doesn't matter.
    $storeList = file_get_contents('https://www.csoki.com/cellwebsite/cellularsales.asp?QueryType=LOCATIONS&Market=KX');

    // If not working, see if there is a manual file to load.
    if ($storeList === false) {
      $path = file_directory_path() . '/cs/store-data.html';
      if (file_exists($path) && $storeList = file_get_contents($path)) {
        watchdog($wd, 'No response from CSOKI; using store-data.html.', array(), WATCHDOG_WARNING);
        unlink($path); // Only process once because if we leave it here it could cause deletions.
      }
    }
  }
  
  // Statistic data for current update.
  $stats = array('new' => 0,'update' => 0,'delete' => 0,'error' => 0,'cnt' => 0);
  if (preg_match('/<error>(.+)<\/error>/', $storeList, $errText) != 0) {
    drupal_set_message($errText[1],'status');
    watchdog($wd, $errText[1], array(), WATCHDOG_ERROR);
    $stats['error']++;
  } else if (!is_bool($storeList) && $storeList!=false && preg_match("/<\/html>/",$storeList)==1) {  // Test if data is received    
    // Initialize helper arrays - index & remove arrays.
    $inx = array('market' => 0,'loc' => 1,'name' => 2,'street' => 3,'city' => 4,'state' => 5,'zip' => 6,'phone' => 7,'fax' => 8,'outlet' => 9,'addr2' => 10,'t_phone' => 11,'storehours' => 12);
    
    preg_match_all('/<datarow>(.*?)<\/datarow>/s', $storeList, $matches, PREG_PATTERN_ORDER);
    $match_count = count($matches[1]);
    unset($matches[0]);  // Don't need this information.
    
    if ($match_count < 1) { // Fix 2012-10-20. If there are no matches in the file all the stores will be deleted!
      $errText = 'File is invalid - no datarow matches found.';
      drupal_set_message($errText, 'status');
      watchdog($wd, $errText, array(), WATCHDOG_ERROR);
      $stats['error']++;
    } else {
      // Zero out "opened" column for processing.
      db_query('UPDATE {cs_store} SET opened = 0 WHERE csoki_update = 1');  // Only updatable. Coming soon stores may be next.
    }
    
    $store_sql = 'SELECT ss.*, l.*, lp.phone AS p_phone, lf.fax FROM {cs_store} ss 
      LEFT OUTER JOIN {location_instance} li ON li.nid = ss.nid 
      LEFT OUTER JOIN {location} l ON l.lid = li.lid 
      LEFT OUTER JOIN {location_phone} lp ON lp.lid = li.lid 
      LEFT OUTER JOIN {location_fax} lf ON lf.lid = li.lid 
      WHERE ss.marketid = :market AND ss.loc = :loc';
    
    // Process store values.
    for ($t = 0; $t < $match_count; $t++) {
      preg_match_all('/<data>(.*?)<\/data>/s',$matches[1][$t],$data,PREG_PATTERN_ORDER);
      unset($data[0]);
      // Filter out matches with no street address OR a name with "special event" OR market = "MG"
      if (trim($data[1][ $inx['street'] ])=="" || strcasecmp(trim($data[1][ $inx['name'] ]),"special event")==0 || trim($data[1][ $inx['market'] ])=="MG") continue;
      // Filter out #B, T#, & IC location codes
      if (preg_match("/\dB|T\d|IC/",$data[1][ $inx['loc'] ])) continue;
      // Replace any single quotes from the data
      $data[1] = str_replace("'","`",$data[1]);
      // Make sure index is uppercase
      $data['market'] = strtoupper($data[1][ $inx['market'] ]);
      $data['loc'] = strtoupper($data[1][ $inx['loc'] ]);
      // Make sure trim all data values (0 & 1 already trimmed)
      $data['outlet'] = trim($data[1][ $inx['outlet'] ]);
      $data['t_phone'] = trim($data[1][ $inx['t_phone'] ]);
      $data['phone'] = trim($data[1][ $inx['phone'] ]);
      $data['fax'] = trim($data[1][ $inx['fax'] ]);
      $data['zip'] = trim($data[1][ $inx['zip'] ]);
      $data['state'] = trim($data[1][ $inx['state'] ]);
      $data['city'] = trim($data[1][ $inx['city'] ]);
      $data['street'] = trim($data[1][ $inx['street'] ]);
      $data['addr2'] = trim($data[1][ $inx['addr2'] ]);
      $data['name'] = trim($data[1][ $inx['name'] ]);
      $data['hours'] = cs_parse_store_hours($data[1][ $inx['storehours'] ]);
      unset($data[1]);
      
      // Store the market value for finding new markets.
      $marketList[] = $data['market'];
      $stats['cnt']++;  // Update the store count.
      
      $results = db_query($store_sql, array(':market' => $data['market'], ':loc' => $data['loc']))->fetchAssoc();
      if (!$results) {
        // NOT in db - INSERT NEW STORE.
        $nid = cs_import_store_node($data, $install);
        // Update the just created "cs_store" entry.
        db_query('UPDATE {cs_store} SET t_phone = :t_phone, hours = :hours, csoki_update = 1 WHERE nid = :nid',
          array(':t_phone' => $data['t_phone'], ':hours' => $data['hours'], ':nid' => $nid));
        // Update the statistics.
        $stats['new']++;
        if (!$install) {
          watchdog($wd, 'New store @market-@loc.', array('@market' => $data['market'], '@loc' => $data['loc']));
        }
      } else {
        // Check if this is a deleted store OR a don't modify store.
        $results = (object)$results;
        if ($results->nid == -1 || $results->csoki_update == 0) {
          // Update "opened" column so it doesn't get deleted.
          db_query('UPDATE {cs_store} SET opened = 1 WHERE nid = :nid', array(':nid' => $results->nid));
          continue;
        }
        $hasChanged = false;
        $updateDesc = "";
        // Has the tracking phone changed?
        if ($data['t_phone'] != $results->t_phone || (!empty($data['t_phone']) && $data['t_phone'] != $results->p_phone)) {
          $hasChanged = true;
          $updateDesc .= 'Tracking phone';
          $pval = empty($data['t_phone']) ? 'NULL' : $data['t_phone'];
          db_query('UPDATE {cs_store} SET t_phone = :t_phone WHERE nid = :nid', array(':t_phone' => $pval, ':nid' => $results->nid));
          if (!empty($data['t_phone'])) {
            if (!empty($results->p_phone)) {
              db_query('UPDATE {location_phone} SET phone = :phone WHERE lid = :lid', array(':phone' => $data['t_phone'], ':lid' => $results->lid));
            } else {
              db_query('DELETE FROM {location_phone} WHERE lid = :lid', array(':lid' => $results->lid));  // Just in-case it is there but empty
              db_query('INSERT INTO {location_phone} (lid, phone) VALUES (:lid, :phone)', array(':lid' => $results->lid, ':phone' => $data['t_phone']));
            }
            // Set variables for next if statement
            $results->p_phone = $data['t_phone'];
          } else {
            // Tracking phone is NULL - delete the location phone (the next if statement will fill it)
            db_query('DELETE FROM {location_phone} WHERE lid = :lid', array(':lid' => $results->lid));
            $results->p_phone = null;
          }
        }
        // Has the normal phone changed?
        if ($data['phone'] != $results->p_phone) {
          $hasChanged = true;
          if (!empty($updateDesc)) $updateDesc .= ', ';
          $updateDesc .= 'Regular Phone';
          db_query('DELETE FROM {location_phone} WHERE lid = :lid', array(':lid' => $results->lid));  // Just in-case it is there but empty
          if (!empty($data['phone'])) {
            db_query('INSERT INTO {location_phone} (lid, phone) VALUES (:lid, :phone)', array(':lid' => $results->lid, ':phone' => $data['phone']));
          }
        }
        // Has the fax changed?
        if ($data['fax'] != $results->fax) {
          $hasChanged = true;
          if (!empty($updateDesc)) $updateDesc .= ', ';
          $updateDesc .= 'Fax';
          db_query('DELETE FROM {location_fax} WHERE lid = :lid', array(':lid' => $results->lid));  // Just in-case it is there but empty
          if (!empty($data['fax'])) {
            db_query('INSERT INTO {location_fax} (lid, fax) VALUES (:lid, :fax)', array(':lid' => $results->lid, ':fax' => $data['fax']));
          }
        }
        // Has the store hours changed?
        if ($data['hours'] != $results->hours) {
          $hasChanged = true;
          if (!empty($updateDesc)) $updateDesc .= ', ';
          $updateDesc .= 'Store hours';
          $pval = empty($data['hours']) ? 'NULL' : $data['hours'];
          db_query('UPDATE {cs_store} SET hours = :hours WHERE nid = :nid', array(':hours' => $pval, ':nid' => $results->nid));
        }
        // Has the outlet, name, or address2 changed?
        if ($data['outlet'] != $results->outlet || $data['name'] != $results->name || $data['addr2'] != $results->additional) {
          $hasChanged = true;
          if ($data['outlet'] != $results->outlet) {
            if (!empty($updateDesc)) $updateDesc .= ', ';
            $updateDesc .= 'Outlet#';
          }
          if ($data['name'] != $results->name) {
            if (!empty($updateDesc)) $updateDesc .= ', ';
            $updateDesc .= 'Store Name';
          }
          if ($data['addr2'] != $results->additional) {
            if (!empty($updateDesc)) $updateDesc .= ', ';
            $updateDesc .= 'Additional directions';
          }
          db_query(
            'UPDATE {cs_store} ss JOIN {location_instance} li ON li.nid=ss.nid JOIN {location} l ON l.lid=li.lid
            SET ss.outlet = :outlet, l.name = :name, l.additional = :addr2
            WHERE ss.nid = :nid',
            array(':outlet' => $data['outlet'], ':name' => $data['name'], ':addr2' => $data['addr2'], ':nid' => $results->nid)
          );
        }
        // Has the address changed?
        if ($data['street']!=$results->street || $data['city']!=$results->city || $data['state']!=$results->province || $data['zip']!=$results->postal_code) {
          $hasChanged = true;
          if ($data['street']!=$results->street) {
            if (!empty($updateDesc)) $updateDesc .= ', ';
            $updateDesc .= 'Street Address';
          }
          if ($data['city']!=$results->city) {
            if (!empty($updateDesc)) $updateDesc .= ', ';
            $updateDesc .= 'City';
          }
          if ($data['state']!=$results->province) {
            if (!empty($updateDesc)) $updateDesc .= ', ';
            $updateDesc .= 'State';
          }
          if ($data['zip']!=$results->postal_code) {
            if (!empty($updateDesc)) $updateDesc .= ', ';
            $updateDesc .= 'Zipcode';
          }
          // Geolocate the address with module's geocoder.
          $addr = cs_geolocate_address($data, true);
          db_query(
            'UPDATE {location} SET street = :street, city = :city, province = :state, postal_code = :zip, latitude = :lat, longitude = :lng
            WHERE lid = :lid',
            array(':street' => $data['street'], ':city' => $data['city'], ':state' => $data['state'], ':zip' => $data['zip'],
              ':lat' => $addr['lat'], ':lng' => $addr['lng'], ':lid' => $results->lid)
          );
          // Now update the geoaccuracy value.
          db_query('UPDATE {cs_store} SET geo_accuracy = :acc WHERE nid = :nid', array(':acc' => $addr['acc'], ':nid' => $results->nid));
        }
        // If anything has changed update the stats and make a log entry.
        if ($hasChanged) {
          $stats['update']++;
          // Make a log entry
          watchdog($wd, 'Updated @market-@loc: @desc.', array('@market' => $data['market'], '@loc' => $data['loc'], '@desc' => $updateDesc));
        }
        // Mark the store as opened!
        db_query('UPDATE {cs_store} SET opened = 1 WHERE nid = :nid', array(':nid' => $results->nid));
      }
    }
  } else {
    $errText = $import_path ? t('Error: No data found in import path: %path.', array('%path' => $import_path)) : t('ERROR: No response from CSOKI.');
    drupal_set_message($errText, 'status');
    watchdog($wd, $errText, array(), WATCHDOG_ERROR);
    $stats['error']++;
    return null;
  }
  // Remove closed stores! ("opened" column set to zero)
  if ($stats['error'] == 0) {
    $results = db_query('SELECT nid, marketid AS market, loc FROM {cs_store} WHERE opened = 0 AND csoki_update = 1');
    foreach ($results as $storeRM) {
      $stats['delete']++;
      // Remove the store node
      node_delete(intval($storeRM->nid));
      // Fully Delete cs_store entry
      db_query('DELETE FROM {cs_store} WHERE marketid = :market AND loc = :loc', array(':market' => $storeRM->market, ':loc' => $storeRM->loc));
      watchdog($wd, 'Deleted @market-@loc.', array('@market' => $storeRM->market, '@loc' => $storeRM->loc));
    }
    db_query('DELETE FROM {cs_store} WHERE opened = 0 AND csoki_update = 1');
  }
  // Update the overall statistics values
  $stats_str = "{$stats['cnt']} stores processed. {$stats['new']} inserted, {$stats['update']} updated, and {$stats['delete']} deleted stores.";
  if ($stats['error'] != 0) {
    $stats_str .= " {$stats['error']} errors.";
  }
  watchdog($wd, $stats_str);
  drupal_set_message($stats_str, 'status');
}


/**
 * Re-geocode every store location. Fills in lat/lng & geo_accuracy columns.
 */
function cs_geocode_all() {
  $results = db_query("
    SELECT CONCAT(ss.marketid, '-', ss.loc) AS storecode, l.lid, l.street, l.city, l.province, l.postal_code FROM {location} l
    INNER JOIN {location_instance} li ON li.lid = l.lid
    INNER JOIN {cs_store} ss ON ss.nid = li.nid
    WHERE ss.csoki_update = 1 AND ss.geo_accuracy <= 0
    ORDER BY ss.marketid, ss.loc");
  $cnt = 0;
  $error_count = 0;
  foreach ($results as $loc) {
    $geocode = cs_geolocate_address((array)$loc, true);
    // Store the lat/lng & accuracy values back to the DB.
    if ($geocode['resultCnt'] == -1) {
      $error_count++;
      if ($error_count > 10) {
        watchdog('cs', t('Exited cs_geocode_all with too many errors.', array(), WATCHDOG_ERROR));
        break;
      }
    } else {
      db_query(
        'UPDATE {location} l INNER JOIN {location_instance} li ON li.lid = l.lid INNER JOIN {cs_store} ss ON ss.nid = li.nid
        SET l.latitude = :lat, l.longitude = :lng, ss.geo_accuracy = :acc 
        WHERE l.lid = :lid',
        array(':lat' => $geocode['lat'], ':lng' => $geocode['lng'], ':acc' => $geocode['acc'], ':lid' => $loc->lid)
      );
      $cnt++;
    }
    if ($cnt == 100) { // Avoid using all geolocation queries. Cron will call this every hour.
      break;
    }
    if ($cnt % 10 == 0) { // They will also kill us if we make requests to fast.
      sleep(3);
    }
  }
  drupal_set_message(t("{$cnt} stores re-geocoded."));
}


/**
 * Display the mailer interface. 
 */
function cs_admin_mailer() {
  $page[] = array('#markup' => 
    "This form takes a CSV (comma delimited) file containing a list of mail addresses.  
    Once the file is processed, it will return a new CSV file with and extra column containing the nearest store id for each address listed based on the zipcode.
    <br /><br />
    USE ONLY DURNING KNOWN SLOW TIMES FOR THE WEBSITE.  
    This function will slow down the website considerably while processing depending on how long the list is.
    <br /><br />");
  $page[] = drupal_get_form('cs_mailer_form');
  return $page;
}


function cs_mailer_form() {
  $form['csv_file'] = array(
    '#type' => 'file',
    '#title' => t('Select the CSV mailer file'),
    '#size' => 60,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Process'),
  );
  $form['#attributes'] = array('enctype' => "multipart/form-data");
  return $form;
}


function cs_mailer_form_submit($form, &$form_state) {
  $validators = array(
    'file_validate_extensions' => array('csv')
  );
  if ($csvfile = file_save_upload('csv_file', $validators)) {
    if (($fp = fopen($csvfile->uri, 'r')) !== false) {
      include_once('./' .drupal_get_path('module','location') . '/earth.inc');  // Use the same distance calculation as the website.
      $newfname = trim(basename($csvfile->filename, '.csv')) . '_done.csv';
      $newfname = str_replace(' ', '_', $newfname);
      
      $zipCol = -1;
      $data = fgetcsv($fp);
      $colCount = count($data);
      $addCol = $colCount + 1;
      if ($data) {
        for($i=0; $i<count($data); $i++) {
          if (strtolower($data[$i]) == 'zip') {
            $zipCol = $i;
            break;
          }
        }
      }
      
      if ($zipCol < 0) {
        form_set_error('', 'No "Zip" column found in file header.');
        fclose($fp);
        return;
      }
      
      $zipStore = array(); // Holds a cache of a zipcode to the nearest store.
      // $stdout = fopen('php://stdout'); // No idea why this won't work.
      $stdout = tmpfile();
      fputcsv($stdout, $data);
      while (($data = fgetcsv($fp)) !== false) {
        // Trim & limit the zipcode to 5 digits.
        $zipcode = substr(trim($data[$zipCol]), 0, 5);
        // Is the zip in the cache.
        if (!isset($zipStore[$zipcode])) {
          // Lookup the zip code geo coordinates in storeloc_zips.
          if (!$zipgeo = db_query('SELECT latitude, longitude FROM {zipcodes} WHERE zip = :zip LIMIT 0,1', array(':zip' => $zipcode))->fetchAssoc()) {
            continue;
          }
          $res = cs_nearest_stores_query($zipgeo['latitude'], $zipgeo['longitude'], $limit = 6);
          $cnt = 0;
          foreach ($res as $store) { //Cache result.
            $zipStore[$zipcode][] = $store->storecode;
            $cnt++;
          }
          if ($cnt == 0) continue; // Note it does not write any rows if none are found.
        }
        
        $data[$colCount] = ''; // Original method always had a blank column.
        for($i=0; $i<count($zipStore[$zipcode]); $i++) {
          $data[$addCol+$i] = $zipStore[$zipcode][$i];
        }
        fputcsv($stdout, $data);
      }
      
      drupal_add_http_header('Pragma', 'no-cache');
      drupal_add_http_header('Expires', 'Mon, 26 Jul 1997 05:00:00 GMT');
      drupal_add_http_header('Last-Modified', gmdate('D, d M Y H:i:s') . ' GMT');
      drupal_add_http_header('Cache-Control', 'must-revalidate, no-store, no-cache, post-check=0, pre-check=0');
      drupal_add_http_header('Content-Transfer-Encoding', 'binary');
      drupal_add_http_header('Content-Disposition', 'attachment; filename="'.$newfname.'"');
      drupal_add_http_header('Content-Type', 'application/octet-stream', true);
      drupal_add_http_header('Content-Type', 'application/octetstream', true);
      drupal_add_http_header('Content-Type', 'application/force-download', true);
      drupal_add_http_header('Content-Type', 'application/download', true);
      
      fclose($fp);
      rewind($stdout); // Should be able to use $stdout directly, but added rewind and fpassthru with temp file as a work-around.
      fpassthru($stdout);
      fclose($stdout);
      exit;
    }
  }
}

/**
 * Create a CSV file with Market, Location, Store Name, Latitude, Longitude for BI Group.
 * 
 */
function cs_location_file() {
  $out = fopen(file_directory_path() . '/BIgroup/store_location.csv', 'w');
  if ($out === false) {
    watchdog('cs', 'Failed to create store_location.csv file.', WATCHDOG_ERROR);
    return false;
  }
  fputcsv($out, array('Market', 'Location', 'Store Name', 'Latitude', 'Longitude'));
  $sql = "SELECT ss.marketid, ss.loc, l.name, l.latitude, l.longitude
    FROM {cs_store} ss
    INNER JOIN {location_instance} i ON i.nid = ss.nid
    INNER JOIN {location} l ON i.lid = l.lid
    WHERE ss.opened <> 0
    ORDER BY ss.marketid, l.name";
  $res = db_query($sql);
  foreach ($res as $store) {
    fputcsv($out, array($store->marketid, $store->loc, $store->name, $store->latitude, $store->longitude));
  }
  fclose($out);
  watchdog('cs', 'Created store_location.csv file');
  return true;
}



/**
 * Display the storeloc log and various statistics.
 * Stats: # stores; # stores with pics; # markets; 
 */
function storeloc_admin_log() {
  // drupal_add_css("./" . drupal_get_path('module','storeloc') . "/storeloc_admin.css" );
  // Define the needed javascript
  $js =  "function storeloc_logClick( logitem, imgElm ) {\n";
  $js .= " itmElm = document.getElementById('sloc_item_'+logitem);\n";
  $js .= " if (itmElm.style.display=='none') { itmElm.style.display='block'; imgElm.src='/".drupal_get_path('module','storeloc')."/images/folder_minus.gif'; }\n";
  $js .= " else { itmElm.style.display='none'; imgElm.src='/".drupal_get_path('module','storeloc')."/images/folder_plus.gif'; }\n";
  $js .= "}\n";
  drupal_add_js($js,"inline");
  $mrktCnt = db_result(db_query("SELECT COUNT(id) FROM {storeloc_market}"));
  $stCnt = db_result(db_query("SELECT COUNT(nid) FROM {cs_store} WHERE nid<>-1"));
  $stNoPicCnt = db_result(db_query("SELECT COUNT(ss.nid) FROM {cs_store} ss JOIN {content_type_store} cs ON cs.nid=ss.nid WHERE ss.nid<>-1 AND cs.field_store_photo_fid IS NOT NULL"));
  $page = "<div id='storeloc_stats'>{$stCnt} stores in {$mrktCnt} markets.  {$stNoPicCnt} stores have photos.</div>";
  $page .= "<div id='storeloc_log'>";
  $results = db_query("SELECT * FROM {storeloc_log} WHERE type<5 ORDER BY stamp DESC, type ASC");
  $cnt = 0;
  while ($storeLst = db_fetch_object($results)) {
    if ($storeLst->type==0) {
      if ($cnt>0) $page .= "</table></div></div>";
      $cnt += 1;
      $page .= "<div class='storeloc_item'><img width='11' height='11' src='/".drupal_get_path('module','storeloc')."/images/folder_plus.gif' class='storeloc_plus' onclick='storeloc_logClick({$cnt},this);' /> ";
      $page .= $storeLst->stamp . ": " . $storeLst->msg;
      $page .= "<br /><div class='storeloc_details' id='sloc_item_{$cnt}' style='display:none;'><table class='storeloc_tbl'>";
    } else {
      $page .= "<tr><td>";
      $stLink = "<a href='/store_location/" . $storeLst->market . "-" . $storeLst->loc."'>".$storeLst->market."-".$storeLst->loc."</a>";
      switch ($storeLst->type) {
        case 1:  // insert
          $page .= "INSERTED {$stLink}";
          break;
        case 2:  // update
          $page .= "UPDATED {$stLink}: " . $storeLst->msg;
          break;
        case 3:  // delete
          $page .= "DELETED {$stLink} (link should not work)";
          break;
        case 4:  // error
          $page .= $storeLst->msg;
          break;
      }
      $page .= "</td></tr>";
    }
  }
  $page .= "</table></div></div></div>";
  // Display the simple stats
  return $page;
}



/**
 * Display the CSOKI store updater interface. 
 */
function storeloc_admin_updater() {
  $page =  "Update the website stores database with CSOKI.<br />";
  $page .= "**Use only durning known slow times for the website**  This function will slow down the website considerably while processing.<br /><br />";
  $page .= drupal_get_form('storeloc_CSOKI_updateForm');
  return $page;
}
function storeloc_CSOKI_updateForm() {
  $form['update_btn'] = array(
    '#type' => 'submit',
    '#value' => t('Update Stores'),
    '#submit' => array('storeloc_CSOKI_update')
  );
  return $form;
}

