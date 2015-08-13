<?php
// $Id$

/**
 * @file
 * Administration page callbacks for the store locator module.
 */
 
/**
 * Display the dealer log and some dealer stats.
 * Stats: # dealers, # markets, # dealers with pics
 */
function dealerportal_admin_log() {
  // drupal_add_css("./" . drupal_get_path('module','dealerportal') . "/dealerportal_admin.css" );
  //Define the needed javascript
  $js =  "function dp_logClick( logitem, imgElm ) {\n";
  $js .= " itmElm = document.getElementById('sloc_item_'+logitem);\n";
  $js .= " if (itmElm.style.display=='none') { itmElm.style.display='block'; imgElm.src='/".drupal_get_path('module','dealerportal')."/images/folder_minus.gif'; }\n";
  $js .= " else { itmElm.style.display='none'; imgElm.src='/".drupal_get_path('module','dealerportal')."/images/folder_plus.gif'; }\n";
  $js .= "}\n";
  drupal_add_js($js,"inline");
  $mrktCnt = db_result(db_query("SELECT COUNT(id) FROM {storeloc_market}"));
  $dlrCnt = db_result(db_query("SELECT COUNT(nid) FROM {dealerportal_dealer} WHERE nid<>-1"));
  $dlrPicCnt = db_result(db_query("SELECT COUNT(dd.nid) FROM {dealerportal_dealer} dd JOIN {content_type_dealer_profile} dp ON dp.nid=dd.nid WHERE dd.nid<>-1 AND dp.field_dealerphoto_fid IS NOT NULL"));
  $page = "<div id='dp_stats'>{$dlrCnt} dealers in {$mrktCnt} markets.  {$dlrPicCnt} dealers have photos.</div>";
  $page .= "<div id='dp_log'>";
  $results = db_query("SELECT d.*, n.title FROM {dealerportal_log} d LEFT OUTER JOIN {dealerportal_dealer} dd ON dd.market=d.market AND dd.dcode=d.dcode LEFT OUTER JOIN {node} n ON n.nid=dd.nid WHERE d.type<5 ORDER BY d.stamp DESC, d.type ASC");
  $cnt = 0;
  while ($dealerLst = db_fetch_object($results)) {
    if ($dealerLst->type==0) {
      if ($cnt>0) $page .= "</table></div></div>";
      $cnt += 1;
      $page .= "<div class='dp_item'><img width='11' height='11' src='/".drupal_get_path('module','dealerportal')."/images/folder_plus.gif' class='dp_plus' onclick='dp_logClick({$cnt},this);' /> ";
      $page .= $dealerLst->stamp . ": " . $dealerLst->msg;
      $page .= "<br /><div class='dp_details' id='sloc_item_{$cnt}' style='display:none;'><table class='dp_tbl'>";
    } else {
      $page .= "<tr><td>";
      $stLink = "<a href='/dealer/".strtolower($dealerLst->market.$dealerLst->dcode)."'>".$dealerLst->title."</a>";
      switch ($dealerLst->type) {
        case 1:  //insert
          $page .= "INSERTED {$stLink}";
          break;
        case 2:  //update
          if (empty($dealerLst->title)) $page .= "UPDATED: " . $dealerLst->msg;
          else $page .= "UPDATED {$stLink}: " . $dealerLst->msg;
          break;
        case 3:  //delete
          $page .= "DELETED: " . $dealerLst->msg;
          break;
        case 4:  //error
          $page .= $dealerLst->msg;
          break;
      }
      $page .= "</td></tr>";
    }
  }
  if ($cnt==0) $page .= "<div class='dp_item'>No data available</div></div>";
  else $page .= "</table></div></div></div>";
  //Display the simple stats
  return $page;
}

/**
 * Used to figure out the market for every in the ubercard order table
 * Use the form button below to call this function, when needed:
 *   $form['orderProcess'] = array('#type' => 'submit','#value' => t('Bulk Process Orders'),'#submit' => array('dealerportal_bulkProcessOrders'));
 */
function dealerportal_bulkProcessOrders()
{
  //include the storeloc module for geolocation
  //include_once('./'. drupal_get_path('module','storeloc') . "/storeloc.module");
  //Since the actual purchase happens at the store, the only meanful order states are: "pending" & "processing"
  $sql = "SELECT o.order_id,o.delivery_street1 AS street,o.delivery_city AS city,z.zone_code AS state,o.delivery_postal_code AS zip FROM {uc_orders} o ";
  $sql.= "INNER JOIN {uc_zones} z ON z.zone_id = o.delivery_zone ";
  $sql.= "INNER JOIN {uc_order_statuses} s ON s.order_status_id = o.order_status ";
  $sql.= "LEFT OUTER JOIN {dealerportal_orderqueue} oq ON oq.orderid = o.order_id ";
  $sql.= "WHERE s.state = 'post_checkout' AND oq.id IS NULL";
  $orderList = db_query($sql);
  while ($order = db_fetch_object($orderList)) {
    //Sanitize the zipcode
    $zipcode = trim($order->zip);
    $zipcode = substr($zipcode,0,5);  //Only want the 5 digit zipcode
    //Geocode the zipcode
    $geozip = db_fetch_object(db_query("SELECT city,state,lat,lng FROM {storeloc_geozip} WHERE zip='%s'",$zipcode));
    if ($geozip) {
      $geocode['lat']=$geozip->lat;
      $geocode['lng']=$geozip->lng;
    } else {
      //Zipcode not found in DB -> Use google's geocoding which can handle wrong zipcodes
      $custaddr = cs_format_geolocation_address($order->street,$order->city,$order->state,$order->zip);
      $geocode = cs_geolocate_address( $custaddr );
      //Ignore multiple return values from google
    }
    //Find the nearest store's market from the customer
    $nearestSt = db_fetch_object(cs_nearest_stores_query($geocode['lat'], $geocode['lng'], 1));
    if (!$nearestSt) continue;  //No stores, skip
    $market = db_result(db_query("SELECT marketid FROM {cs_store} WHERE nid=%d",$nearestSt->nid));
    //Create a "dealerportal_orderqueue" entry; notified is set to disable notifications
    $sql = "INSERT INTO {dealerportal_orderqueue} (market,orderid,lat,lng,zip,notified) VALUES ('{$market}',{$order->order_id},{$geocode['lat']},{$geocode['lng']},{$zipcode},NOW())";
    db_query($sql);
  }
}

/**
 * Display the CSOKI dealer updater interface. 
 */
function dealerportal_admin_updater() {
  $page =  "Update the website dealer database with CSOKI.<br />";
  $page .= "**Use only durning known slow times for the website**  This function will slow down the website considerably while processing.<br /><br />";
  $page .= drupal_get_form('dealerportal_CSOKI_updateForm');
  return $page;
}
function dealerportal_CSOKI_updateForm() {
  $form['update_btn'] = array(
    '#type' => 'submit',
    '#value' => t('Update Dealers'),
    '#submit' => array('dealerportal_CSOKI_update')
  );
  return $form;
}
/**
 * Sync dealer database with CSOKI. 
 */
function dealerportal_CSOKI_update() {
  //Clear out "dealerportal_log" for current date; don't want overlapping data
  db_query("DELETE FROM {dealerportal_log} WHERE stamp=CURDATE() AND type<5");
  //Remove log entries more than 15 days old
  db_query("DELETE FROM {dealerportal_log} WHERE stamp<DATE_SUB(CURDATE(),INTERVAL 15 DAY) AND type<5");
  //Zero out the "is_valid" column for processing
  db_query("UPDATE {dealerportal_dealer} SET is_valid=0");
  
  $createDealers = false;  //Allow the creation of new dealers, if found; Initially just want to update the beta testers
  
  //Must loop through all on the markets; "All in one" option not available
  //Create a list of the available markets
  if ($createDealers==false) $sql = "SELECT DISTINCT market FROM {dealerportal_dealer}";  //Only markets of the dealer's is stored
  else $sql = "SELECT id AS market FROM {storeloc_market}";
  $results = db_query($sql);
  $mrktList = null;
  while ($mrktVal = db_fetch_object($results)) {
    $mrktList[]=$mrktVal->market;
  }
  //Add markets: "TR" (Training) & "MG" (Management) as requested by Jay Witherspoon
  if ($createDealers==true) {
    if (!in_array("MG",$mrktList)) $mrktList[]="MG";
    if (!in_array("TR",$mrktList)) $mrktList[]="TR";
  }
  
  include_once('./'. drupal_get_path('module', 'node') .'/node.pages.inc');
  //Initialize helper arrays - index & remove arrays
  $inx = array('market'=>0,'dcode'=>1,'name'=>2,'cell'=>4,'email'=>5);
  $stats = array('new'=>0,'update'=>0,'delete'=>0,'error'=>0,'cnt'=>0);  //Statistic data for current update
  //Setup the new user's dealer role (never changes)
  $dRoleId = db_result(db_query("SELECT rid FROM {role} WHERE name='dealer' LIMIT 1"));
  $dRoleArr = array();
  $dRoleArr[$dRoleId] = 'dealer';
  
  //Loop through all of the markets
  for($z=0;$z<count($mrktList);$z++) {
    //Load the "Dealer List" data!
    $dealerList = file_get_contents('https://www.csoki.com/cellwebsite/cellularsales.asp?QueryType=DEALERLIST&Market=' . $mrktList[$z]);
    //$dealerList = file_get_contents('http://cellsales2.hm/sites/all/modules/SCM_modules/dealerportal/DealerList_AL.html');
    
    //Look for errors
    if (preg_match("/<error>(.+)<\/error>/",$dealerList,$errText)!=0) {
      drupal_set_message("DEALER LIST ".$errText[1],'status');
      db_query("INSERT INTO {dealerportal_log} VALUES ('%s',NULL,'%s',CURDATE(),4)",$mrktList[$z],"DEALER LIST ".$errText[1]);  //Type 4 = ERROR
      $stats['error']++;
      if (strncasecmp($errText[1],"ERROR: INVALID IP OF CALLER",27)==0) break; //No need for more processing
    }
    //Test if data is received
    if (!is_bool($dealerList) && $dealerList!=false && preg_match("/<\/html>/",$dealerList)==1) {
      //Data received -> Process dealer values
      preg_match_all("/<datarow>(.*?)<\/datarow>/",$dealerList,$matches,PREG_PATTERN_ORDER);
      unset($matches[0]);  //Don't need this information
      for($t=0;$t<count($matches[1]);$t++) {
        preg_match_all("/<data>(.*?)<\/data>/",$matches[1][$t],$data,PREG_PATTERN_ORDER);
        unset($data[0]);
        //Replace any single quotes from the data
        $data[1] = str_replace("'","`",$data[1]);
        //Make sure trim all data values
        $data['name'] = trim($data[1][ $inx['name'] ]);
        $data['email'] = trim($data[1][ $inx['email'] ]);
        //Filter out fake dealers - "TRADESHOW", "EQUIPMENT", "TRAINING", One word names, 3 names without email
        $nameCnt = count(preg_split("/[\s]+/",$data['name']));
        if ($nameCnt==1 || ($nameCnt>2 && empty($data['email']))) continue;
        if (preg_match("/^TRADE/i",$data['name']) || preg_match("/^TRAIN/i",$data['name']) || strncasecmp($data['name'],"EQUIPMENT",9)==0) continue;
        //Create the email address, if missing for 2 named dealers
        if ($nameCnt==2 && empty($data['email'])) $data['email'] = str_replace(" ",".",$data['name']);
        //Add the "@cellularsales.com" to the email address
        $data['alias'] = strtolower(str_replace('.','_',$data['email']));
        $data['email'] = strtolower($data['email']) . "@cellularsales.com";
        //$data['market'] = trim(strtoupper($data[1][ $inx['market'] ]));  //No longer care about this column - redundant
        //The market is now included in the dealer code
        $tmp = trim($data[1][ $inx['dcode'] ]);  
        $data['market'] = strtoupper(substr($tmp,0,2));
        $data['dcode'] = substr($tmp,2);
        $data['cell'] = trim($data[1][ $inx['cell'] ]);
        unset($data[1]);
        $stats['cnt']++;  //Update the dealer count
        //Is this dealer already in the DB?
        $sql = "SELECT dd.cellphone,u.name,u.mail,n.title FROM {dealerportal_dealer} dd INNER JOIN {users} u ON dd.uid=u.uid INNER JOIN {node} n ON dd.nid=n.nid WHERE dd.market='%s' AND dd.dcode=%d";
        if (!$results = db_fetch_object(db_query($sql,$data['market'],intval($data['dcode'])))) {
          //NOT IN DB - INSERT NEW DEALER
          if ($createDealers==false) continue;
          //Create the dealer user - This also creates the "dealerportal_dealer" entry
          $dUser = user_save(new stdClass(),array(
            'name' => $data['market'] . $data['dcode'],
            'pass' => 'use_csoki_pwd_!!',
            'mail' => $data['email'],
            'roles' => $dRoleArr,
            'status' => 1
          ));
          //The dealer profile node && the "dealerportal_dealer" entry is already created with the user insert
          //Update the dealer's cellphone number & his full name
          $sql = "UPDATE {dealerportal_dealer} dd INNER JOIN {node} n ON n.nid=dd.nid INNER JOIN {node_revisions} nr ON nr.vid=n.vid SET dd.cellphone='%s', n.title='%s', nr.title='%s' WHERE market='%s' AND dcode=%d";
          db_query($sql,$data['cell'],$data['name'],$data['name'],$data['market'],intval($data['dcode']));
          //Create the dealer URL alias
          path_set_alias("expert/".strtolower($data['market']).$data['dcode'],$data['alias']);
          //Send out an email to the new dealer!
          $data['username'] = $data['market'] . $data['dcode'];
          drupal_mail('dealerportal', 'new_dealer', $data['email'], language_default(), $data, variable_get('site_mail', 'info@cellularsales.com'));
          //Update the stats
          $stats['new']++;
          db_query("INSERT INTO {dealerportal_log} VALUES ('{$data['market']}','{$data['dcode']}',NULL,CURDATE(),1)");  //Type 1 = new
        } else {
          //IN DB - update dealer
          $hasChanged = false;
          $updateDesc = "";
          //Has the name changed?
          if (strcasecmp($data['name'],$results->title)!=0) {
            $hasChanged = true;
            $updateDesc .= "Name";
            //Update both the node and node_revision title (name) entries
            db_query("UPDATE {node} n INNER JOIN {dealerportal_dealer} dd ON dd.nid=n.nid INNER JOIN {node_revisions} nv ON n.nid=nv.nid AND n.vid=nv.vid SET n.title='%s', nv.title='%s' WHERE dd.market='%s' AND dd.dcode=%d",$data['name'],$data['name'],$data['market'],intval($data['dcode']));
          }
          //Has the username changed?
          if (strcasecmp($data['market'].$data['dcode'],$results->name)!=0) {
            $hasChanged = true;
            if (!empty($updateDesc)) $updateDesc .= ", ";
            $updateDesc .= "Username";
            db_query("UPDATE {users} u INNER JOIN {dealerportal_dealer} dd ON dd.uid=u.uid SET u.name='%s' WHERE dd.market='%s' AND dcode=%d",$data['market'].$data['dcode'],$data['market'],intval($data['dcode']));
          }
          //Has the email changed?
          if (strcasecmp($data['email'],$results->mail)!=0) {
            $hasChanged = true;
            if (!empty($updateDesc)) $updateDesc .= ", ";
            $updateDesc .= "Email, Alias";
            db_query("UPDATE {users} u INNER JOIN {dealerportal_dealer} dd ON dd.uid=u.uid SET u.mail='%s' WHERE dd.market='%s' AND dcode=%d",$data['email'],$data['market'],intval($data['dcode']));
            //Delete old alias
            $old_alias = str_replace(".","_",substr($results->mail,0,strpos($results->mail,'@')));
            db_query("DELETE FROM {url_alias} WHERE dst='%s'",$old_alias);
            //Create the new alias
            path_set_alias("expert/".strtolower($data['market']).$data['dcode'],$data['alias']);
          }
          //Has the cellphone changed?
          if (strcasecmp($data['cell'],$results->cellphone)!=0) {
            $hasChanged = true;
            if (!empty($updateDesc)) $updateDesc .= ", ";
            $updateDesc .= "Cellphone #";
            db_query("UPDATE {dealerportal_dealer} SET cellphone='%s' WHERE market='%s' AND dcode=%d",$data['cell'],$data['market'],intval($data['dcode']));
          }
          //if anything has changed update the stats and make a log entry
          if ($hasChanged==true) {
            $stats['update']++;
            //Make a log entry
            db_query("INSERT INTO {dealerportal_log} VALUES ('{$data['market']}','{$data['dcode']}','{$updateDesc}',CURDATE(),2)");  //Type 2 = update
          }
          //Mark the dealer as valid!
          db_query("UPDATE {dealerportal_dealer} SET is_valid=1 WHERE market='%s' AND dcode=%d",$data['market'],intval($data['dcode']));
        }
      }
    } else {
      //Error: no connection!
      $errText = "DEALER LIST ERROR: No response from CSOKI";
      drupal_set_message($errText,'status');
      db_query("INSERT INTO {dealerportal_log} VALUES ('%s',NULL,'%s',CURDATE(),4)",$mrktList[$t],$errText);  //Type 4 = ERROR
      $stats['error']++;
      break;  //No need to continue
    }
  }
  //Remove fired dealers! ("is_valid" column set to zero)
  //Don't delete dealers when there is an error!
  if ($createDealers==true && $stats['error']==0) {
    $results = db_query("SELECT dd.market,dd.dcode,dd.nid,dd.uid,n.title AS `name` FROM {dealerportal_dealer} dd INNER JOIN {node} n ON dd.nid=n.nid WHERE is_valid=0");
    while ($dealerRM = db_fetch_object($results)) {
      $stats['delete']++;
      //Remove the dealer node
      node_delete(intval($dealerRM->nid));
      //Remove the dealer user node
      user_delete(null,intval($dealerRM->uid));
      //Remove the "dealerportal_dealer" entry - probably already deleted...
      db_query("DELETE FROM {dealerportal_dealer} WHERE market='%s' AND dcode=%d",$dealerRM->market,intval($dealerRM->dcode));
      //Remove the dealer URL alias
      path_set_alias("expert/".strtolower($dealerRM->market).$dealerRM->dcode);
      //Make a log entry
      db_query("INSERT INTO {dealerportal_dealer} VALUES ('{$dealerRM->market}','{$dealerRM->dcode}','{$dealerRM->name}',CURDATE(),3)");  //Type 3 = delete
    }
    db_query("DELETE FROM {dealerportal_dealer} WHERE is_valid=0");  //Remove all the dealer's at once
  }
  //Clear the CSOKI dealer list data
  unset($dealerList);
  
  //***Process the "Roster List"***
  //Remove the market "MG" from the list. Not available
  if (in_array("MG",$mrktList)) unset($mrktList[ array_search("MG",$mrktList) ]);
  //Initialize helper arrays - index & remove arrays
  $inx = array('market'=>0,'date'=>1,'store'=>2,'start'=>3,'end'=>4,'dcode'=>5);
  //Loop through all of the markets for the "Roster List"
  for($z=0;$z<count($mrktList);$z++) {
    //Load the data
    $rosterList = file_get_contents('https://www.csoki.com/cellwebsite/cellularsales.asp?QueryType=ROSTERLIST&Market='.$mrktList[$z].'&Date1='.date("n/j/Y").'&Date2='.date("n/j/Y"));
    //$rosterList = file_get_contents('http://cellsales2.hm/sites/all/modules/SCM_modules/dealerportal/RosterList_AL.html');
    
    //Look for errors
    if (preg_match("/<error>(.+)<\/error>/",$rosterList,$errText)!=0) {
      drupal_set_message("DEALER ROSTER ".$errText[1],'status');
      db_query("INSERT INTO {dealerportal_log} VALUES ('%s',NULL,'%s',CURDATE(),4)",$mrktList[$z],"DEALER ROSTER ".$errText[1]);  //Type 4 = ERROR
      $stats['error']++;
      if (strncasecmp($errText[1],"ERROR: INVALID IP OF CALLER",27)==0) break; //No need for more processing
    }
    //Test if data is received
    if (!is_bool($rosterList) && $rosterList!=false && preg_match("/<\/html>/",$rosterList)==1) {
      //Clear out market's dealers current store assignment and hours; Don't want to see a non-working dealer assigned somewhere!!
      $sql = "UPDATE {content_type_dealer_profile} dp INNER JOIN {dealerportal_dealer} dd ON dd.nid=dp.nid SET field_storelocation_nid=NULL, field_dp_worktime_value=NULL WHERE dd.market='%s'";
      db_query($sql, $mrktList[$z]);
      //Data received -> Process dealer roster values
      preg_match_all("/<datarow>(.*?)<\/datarow>/",$rosterList,$matches,PREG_PATTERN_ORDER);
      unset($matches[0]);  //Don't need this information
      for($t=0;$t<count($matches[1]);$t++) {
        preg_match_all("/<data>(.*?)<\/data>/",$matches[1][$t],$data,PREG_PATTERN_ORDER);
        unset($data[0]);
        //Replace any single quotes from the data
        $data[1] = str_replace("'","`",$data[1]);
        //Make sure trim all data values
        foreach($inx as $key => $val) {
          $data[$key] = trim($data[1][ $inx[$key] ]);
        }
        //Dealer code now contains the dealer's market code
        $data['dMarket'] = strtoupper(substr($data['dcode'],0,2));
        $data['dcode'] = substr($data['dcode'],2);
        unset($data[1]);
        //Is this dealer AND store in the DB?
        $dealerNID = db_result(db_query("SELECT nid FROM {dealerportal_dealer} WHERE market='%s' AND dcode=%d",$data['dMarket'],intval($data['dcode'])));
        if($dealerNID) {
          //Update the dealer's location & work hours
          $storeNID = db_result(db_query("SELECT nid FROM {cs_store} WHERE marketid='%s' AND loc='%s'",$data['market'],$data['store']));
          $wrktime = $data['start'].' - '.$data['end'];
          if ($storeNID) {
            db_query("UPDATE {content_type_dealer_profile} SET field_storelocation_nid=%d, field_dp_worktime_value='%s' WHERE nid=%d",$storeNID,$wrktime,$dealerNID);
          } else {
            db_query("UPDATE {content_type_dealer_profile} SET field_dp_worktime_value='%s' WHERE nid=%d",$wrktime,$dealerNID);
          }
        }
      }
    } else {
      //Error: no connection!
      $errText = "DEALER ROSTER ERROR: No response from CSOKI";
      drupal_set_message($errText,'status');
      db_query("INSERT INTO {dealerportal_log} VALUES ('%s',NULL,'%s',CURDATE(),4)",$mrktList[$t],$errText);  //Type 4 = ERROR
      $stats['error']++;
      break;  //No need to continue
    }
  }
  
  //Update the overall statistics values ("Roster List" has only error stats)
  $stats_str = "{$stats['cnt']} dealers processed. {$stats['new']} inserted, {$stats['update']} updated, and {$stats['delete']} deleted dealers.";
  if ($stats['error']!=0) $stats_str .= " {$stats['error']} errors.";
  db_query("INSERT INTO {dealerportal_log} VALUES (NULL,NULL,'{$stats_str}',CURDATE(),0)");  //Type 0 = stats
  drupal_set_message($stats_str,'status');
}

/**
 * Handle submission of the form and inserting the test data
 * NOT USED - this is for reference only
 */
function dealerportal_admin_settings_submit($form, $form_state) {
  $insertData = $form_state['values']['include'];
  variable_set('dealerportal_has_testdata',$insertData);
  if (!$insertData['testdata']) {
    //Remove test data
    //Only have to remove the users; the "content_profile" module will remove the profile nodes
    $count = 0;
    $sql =  "SELECT u.uid, u.name FROM {users} u ";
    $sql .= "INNER JOIN {users_roles} ur ON ur.uid = u.uid ";
    $sql .= "INNER JOIN {role} r ON ur.rid = r.rid AND r.name = 'dealer' ";
    $sql .= "INNER JOIN {node} n ON n.uid = u.uid AND n.type = 'dealer_profile' ";
    $sql .= "INNER JOIN {node_revisions} nr ON n.nid = nr.nid AND n.vid = nr.vid AND nr.body like 'test_dealer%'";
    $result = db_query($sql);
    while ($userid = db_fetch_object($result)) {
      user_delete(array(), $userid->uid);
      $count++;
    }
    drupal_set_message(t('Removed %cnt test dealer user(s).', array('%cnt' => $count)));
  } else {
    //Check to see if test data already inserted
    $result = db_query("SELECT COUNT(nid) AS 'Num' FROM {node} WHERE type='dealer_profile' AND title='test_dealer'");
    $NumTestStores = db_fetch_object($result);
    if ($NumTestStores->Num > 0) {
      drupal_set_message(t('%cnt test store(s) already inserted.', array('%cnt' => $NumTestStores->Num)));
      return;
    }
    //Initialize test data
    include_once('./'. drupal_get_path('module', 'node') .'/node.pages.inc');
    $dRoleId = db_result(db_query("SELECT rid FROM {role} WHERE name='dealer' LIMIT 1"));
    $dRoleArr = array();
    $dRoleArr[$dRoleId] = 'dealer';
    $testUsers = explode(",","Dave Willow|dwillow@yahoo.com,Brian Eclair|bclair@hotmail.com,Jane Summers|jesummers@msn.com,Betty White|bgwhite@cnn.com,Mark Stone|marks@nytimes.com,Chris Farley|farley@tty.com");
    $storeResult = db_query("SELECT nid, title FROM {node} WHERE type='store' AND status=1");
    $phoneResult = db_query("SELECT n.nid, n.title FROM {node} n INNER JOIN {content_type_product} p ON n.nid=p.nid WHERE n.status=1");
    //Insert test data
    foreach($testUsers as $tuser) {
      $tuParts = explode("|",$tuser);
      $uName = explode("@",$tuParts[1]);
      //Create the user
      user_save(new stdClass(),array(
        'name' => $uName[0],
        'pass' => 'pwd1',
        'mail' => $tuParts[1],
        'roles' => $dRoleArr,
        'status' => 1
      ));
      //Find the created user's id
      $userID = db_result(db_query("SELECT uid FROM {users} WHERE name = '{$uName[0]}' AND mail = '{$tuParts[1]}' LIMIT 1"));
      //Create the user's node profile
      $node = new stdClass();
      $node->type = 'dealer_profile';
      node_object_prepare($node);
      $node->uid = $userID;
      $node->title = $tuParts[0];
      $node->body = 'test_dealer Proin a tellus est, sit amet tincidunt nunc. Curabitur rutrum, ante a commodo hendrerit, erat tellus porttitor massa';
      //insert the next cell phone product in the database
      if (!$phoneID = db_fetch_object($phoneResult)) { reset($phoneResult); $phoneID = db_fetch_object($phoneResult); }
      $node->field_cellproduct[0]['nid'] = $phoneID->nid;
      //insert the next store from the db
      if (!$storeID = db_fetch_object($storeResult)) { reset($storeResult); $storeID = db_fetch_object($storeResult); }
      $node->field_storelocation[0]['nid'] = $storeID->nid;
      $node->field_portal[0]['value'] = rand(0,1)==1?"1":"0";
      node_save($node);
    }
    drupal_set_message(t('Inserted %cnt test dealer user(s).', array('%cnt' => count($testUsers))));
  }
}