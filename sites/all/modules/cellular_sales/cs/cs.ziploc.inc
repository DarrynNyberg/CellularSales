<?php

/**
 * @file
 * Utility for building a file zipcodes with the closest store to each one.
 */


/**
 * Return the nearest store to the location.
 * 
 * Result looks like:
 * 
 * stdClass Object ( [nid] => 4149 [storecode] => ny_ht [lid] => 1176 [name] => HUNTINGTON NY 
 * [street] => 309 MAIN STREET [additional] => [city] => HUNTINGTON [province] => NY [postal_code] => 11743 [country] => us 
 * [latitude] => 40.883709 [longitude] => -73.383802 [source] => 1 [is_primary] => 0 [weeks_old] => 28 [phone] => 631-425-5106 
 * [filepath] => [geo] => 8 [t_phone] => [hours] => Mon-Sat 9:00am-9:00pm Sun 11:00am-6:00pm [distance] => 28.4 )
 * 
 */
function storeloc_nearest_store($lat, $lon=null) {
  if ($lat && is_string($lat) && !$lon) { // allow a string: "lat, long"
    $split = explode(',', $lat);
    if (count($split) == 2) {
      $lat = $split[0];
      $lon = $split[1];
    }
  }
  $res = cs_nearest_stores_query($lat, $lon);
  if (!$res) {
    return null;
  } else {
    $obj = db_fetch_object($res);
    $obj->distance = round($obj->distance * METER_2_MI, 1); // Convert to miles
    return $obj;
  }
}


/** 
 * For the zipcodes provided in sites/files/zipcode_request.txt, write nearest store information.
 * 
 * NOTE: The zipcode location data must be installed in mysql. See location module INSTALL.txt.
 */
function storeloc_stores_nearest_zipcode_file() {
  $req = file_get_contents(file_directory_path() . '/zipcode_request.txt');
  if(!$req) {
    return t('No request data found.');
  }
  $req = explode("\n", $req);
  $out = fopen(file_directory_path() . '/zipcode_results.csv', 'w');
  fputcsv($out, array('Zip', 'Store Name', 'Address', 'Distance'));
  $find = array('postal_code' => '', 'country' => 'us');
  foreach($req as $zip) {
    $store = null;
    $find['postal_code'] = $zip;
    if ($loc = location_get_postalcode_data($find)) {
      $store = storeloc_nearest_store($loc['lat'], $loc['lon']);
    }
    if ($store) {
      $addr = $store->street . ', ' . $store->city . ', ' . $store->province . ' ' . $store->postal_code;
      $line = array($zip, $store->name, $addr, $store->distance);
    } else {
      $line = array($zip, 'Invalid zip code', '', '');
    }
    fputcsv($out, $line);
  }
  fclose($out);
  return count($req) . t(' zipcodes processed.');
}

