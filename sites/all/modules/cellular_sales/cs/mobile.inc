<?php

/**
 * @file
 * Mobile site pages and features.
 */


/**
 * Mobile site home page.
 */
function cs_home_mobile_page() {
  drupal_set_title('');
  $map = isset($_COOKIE['cs_map_view']) && $_COOKIE['cs_map_view'];
  $cookie = cs_location_cookie();
  if (!$cookie) {
    $map = false; // Current javascript updating only does the store list. Could do both, if needed.
    cs_nearest_store_by_ip();
    $cookie = cs_location_cookie();
  }
  $nearest = null;
  $lat = $cookie['lat'];
  $lng = $cookie['lng'];
  $map_from = $lat . ',' . $lng;
  $nearby_stores = cs_nearest_stores_query($lat, $lng);
  foreach ($nearby_stores as $store) {
    $all_stores[] = $store;
    if (!$nearest) {
      $nearest = $store;
    } else {
      $other_stores[] = $store;
    }
  }

  $ra[] = array('#markup' => t('Your local store'), '#prefix' => '<div id="cs-your-local-store"><h1>', '#suffix' => '</h1>');
  
  $ra['nearest_store'] = array(
    '#prefix' => '<div id="cs-nearest-store">', '#suffix' => '</div>',
    'nearest' => cs_store_markup($nearest, '', false, $map_from),
    'navigate' => array('#markup' => cs_map_store_url($nearest, t('Navigate'), $map_from), '#prefix' => '<div id="cs-map-store">', '#suffix' => '</div>'),
    'call' => array('#markup' => cs_phone_store_url($nearest, t('Call store')), '#prefix' => '<div id="cs-call-store">', '#suffix' => '</div>'),
  );
  $ra[] = drupal_get_form('cs_store_locator_mobile_form');
  
  $ra[] = array('#markup' => '</div>'); // id=cs-your-local-store
  
  $js = array(drupal_get_path('module', 'cs') . '/mobile.js');
  $view_form = drupal_get_form($map ? 'cs_list_view_mobile_form' : 'cs_map_view_mobile_form');
  if ($map) {
    $ra['store_map'] = cs_store_map_markup($all_stores);
    $ra[] = array('view_form' => $view_form, '#prefix' => '<div id="cs-view-form" style="clear: both; float: right;">', '#suffix' => '</div>');
  } else {
    $ra[] = array('view_form' => $view_form, '#prefix' => '<div id="cs-view-form" style="float: right;">', '#suffix' => '</div>');
    $ra[] = array('#markup' => t('More stores near you'), '#prefix' => '<h3>', '#suffix' => '</h3>');
    $ra['store_list'] = cs_store_list_markup($other_stores, $map_from);
    $js['http://maps.google.com/maps/api/js?sensor=true'] = array('type' => 'external'); // This already included if map is used.
   
  }
  
  $ra['#attached'] = array('js' => $js);
  return $ra;
}


/**
 * Ajax callback to update the home page for a new location.
 */
function cs_home_mobile_page_update() {
  $location = drupal_get_query_parameters();
  if ($location) { // See if they sent a location from browser or GPS.
    cs_location_cookie_from_geolocation($location, true);
  }
  
  $ra = cs_home_mobile_page();
  $items = array('nearest_store', 'store_list');
  foreach ($items as $key) {
    if (isset($ra[$key])) {
      $item = $ra[$key];
      unset($item['#prefix']); // Remove the surrounding div to get the inner content only.
      unset($item['#suffix']);
      $out[$key] = drupal_render($item);
    }
  }
  // Not drupal_json_output using because it escapes all the HTML formatting.
  // drupal_json_output($out);
  drupal_add_http_header('Content-Type', 'application/json');
  echo json_encode($out);
}



/**
 * Format a list of nearest stores.
 */
function cs_store_list_markup($stores, $map_link = false) {
  $ra = array('#prefix' => '<div id="cs-store-list">', '#suffix' => '</div>');
  foreach($stores as $store) {
    $ra[] = cs_store_markup($store, null, false, $map_link);
  }
  return $ra;
}


/**
 * Display stores on a map. Assumes first store is nearest store.
 */
function cs_store_map_markup($stores) {
  $markers = cs_init_location_markers();
  $count = 0;
  foreach($stores as $store) {
    $count++;
    $icon = "http://chart.apis.google.com/chart?chst=d_map_pin_letter&chld=" . urlencode(chr($count+64) . "|FF766A|000000");
    $markers[] = array(
      'nid' => $store->nid,
      'icon' => $icon,
      'html' => drupal_render(cs_store_markup($store, 'cs_store_thumbnail')),
      'lon' => $store->longitude,
      'lat' => $store->latitude,
      'title' => $store->name,
      'distance' => round($store->distance * METER_2_MI, 1),
    );
  }
  return array('#markup' => cs_add_map($markers, 10), '#prefix' => '<div id="cs-store-map">', '#suffix' => '</div>');
}



/**
 * Form to get address for nearest stores.
 */
function cs_store_locator_mobile_form() {
  $form['new_location'] = array(
    '#title' => t('Change location'),
    '#type' => 'textfield',
    '#size' => 50,
    '#required' => true,
    '#description' => t('Enter your zip code or full street address.'),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Search'),
  );
  return $form;
}


/**
 * Submit handler for store location form.
 */
function cs_store_locator_mobile_form_submit($form, &$form_state) {
  setcookie('cs_user_location', '1', 0, '/'); // Don't request geolocation after user entered something.
  $geocode = cs_geolocate_address(trim($form_state['values']['new_location']));
  $addr = cs_location_cookie_from_geolocation($geocode, false, false);
}


/**
 * Forms and submit handler to swtich between list and map view.
 */
function cs_map_view_mobile_form() {
  $form['submit'] = array('#type' => 'submit', '#value' => t('See on map'));
  return $form;
}

function cs_map_view_mobile_form_submit($form, &$form_state) {
  setcookie('cs_map_view', '1', 0, '/');
}

function cs_list_view_mobile_form() {
  $form['submit'] = array('#type' => 'submit', '#value' => t('See as list'));
  return $form;
}

function cs_list_view_mobile_form_submit($form, &$form_state) {
  setcookie('cs_map_view', '', 0, '/');
}

