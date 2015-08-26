<?php

/**
 * @file
 * Hooks used to alter the operation of non-core modules.
 */


/**
 * Implementation of the hook_form_FORM_ID_alter() for the "webform" module on the "Contact Us" page (node 578).
 */
function cs_form_webform_client_form_578_alter(&$form, &$form_state) {
  $default = $form['submitted']['preferred_store']['#default_value'];
  $options = $form['submitted']['preferred_store']['#options'];
  if ($default == '0' && count($options) <= 1) { // '0' Is the code for 'Other' in the form settings.
    if (!$cookie = cs_location_cookie()) {
      cs_nearest_store_by_ip();
      $cookie = cs_location_cookie();
    }
    $lat = $cookie['lat'];
    $lon = $cookie['lng'];
    $nearest4 = cs_nearest_stores_query($lat, $lon, 4);
    $form['submitted']['preferred_store']['#options'] = array();
    $codes = array();
    foreach ($nearest4 as $store) {
      $codes[] = $store->storecode;
      $form['submitted']['preferred_store']['#options'][$store->storecode] = ucwords(strtolower($store->name));
    }
    $form['submitted']['preferred_store']['#options']['0'] = t('Other');
    if ($codes) {
      $form['submitted']['preferred_store']['#default_value'] = $codes[0];
    }
  } else if ($default != '0' && !array_key_exists($default, $options)) { // User has filled in form but admin can go back and edit.
    $form['submitted']['preferred_store']['#options'] = array($default => $default);
  }
}


/**
 * Implements hook_theme().
 */
function cs_theme() {
  return array(
    'cs_rebate' => array(
      'variables' => array('rebate' => 0),
    ),
  );
}


/**
 * Theme for cs rebate, similar to theme_uc_price.
 */
function theme_cs_rebate($variables) {
  if (isset($variables['rebate']) &&  $variables['rebate'] > 0) {
    return '<span class="cs-rebate">-$' . number_format($variables['rebate'], 2) . '</span>';
  } else {
    return '';
  }
}


/**
 * Return the product rebate for the node, if any.
 */
function cs_product_rebate($node) {
  if ($node && isset($node->list_price) && isset($node->sell_price) && $node->list_price > $node->sell_price) {
    return round($node->list_price - $node->sell_price, 2);
  } else {
    return 0;
  }
}



/**
 * Implements hook_entity_view_alter().
 *
 * Handle Ubercart rebates and other adjustments.
 */
function cs_entity_view_alter(&$build, $entity_type) {
  switch ($entity_type) {
    case 'node':
      // This hook can be called in any context and the uc_product module may not be loaded; so we bail if the function does not exist.
      if (function_exists('uc_product_is_product') && uc_product_is_product($build['#node'])) {
        // Should not have to do this rounding, but the theme developer grabs these values directly instead of using theme functions.
        $build['#node']->price = number_format($build['#node']->price, 2);
        $build['#node']->list_price = number_format($build['#node']->list_price, 2);
        $build['#node']->sell_price = number_format($build['#node']->sell_price, 2);
        
        foreach(array('cost', 'list_price', 'sell_price') as $key) {
          if (isset($build[$key]['#value'])) {
            $build[$key]['#value'] = number_format($build[$key]['#value'], 2);
          }
        }
        
        $build['#node']->rebate = cs_product_rebate($build['#node']);
        $build['rebate'] = array(
          '#theme' => 'cs_rebate',
          '#title' => 'Rebate:',
          '#value' => $build['#node']->rebate,
          '#attributes' => array('class' => array('rebate')),
        );        
        $build['display_price']['#value'] = round($build['display_price']['#value'], 2);
        $build['#node']->display_price = $build['display_price']['#value'];
      }
      break;

    case 'uc_cart_item':
      /*
      $rebate = isset($build['#entity']) ? cs_product_rebate($build['#entity']) : 0;
      if ($rebate > 0 && !empty($build['#total'])) {
        $build['#total'] -= $rebate * $build['qty']['#default_value'];
      }
      */
      break;
    
    case 'uc_order_product':
      $rebate = isset($build['#entity']) ? cs_product_rebate($build['#entity']) : 0;
      $build['rebate'] = array('#theme' => 'cs_rebate', '#rebate' => $rebate, '#cell_attributes' => array('class' => array('rebate')));
      $build['price']['#price'] += $rebate;
      break;
  }
}



/**
 * Implements hook_uc_checkout_pane().
 */
function cs_uc_checkout_pane() {
  $panes['contact'] = array(
    'callback' => 'cs_uc_checkout_pane_contact',
    'title' => t('Contact information'),
    'desc' => t('Cellular Sales custom checkout pane with minimal contact info.'),
    'weight' => 2,
    'process' => true,
    'collapsible' => false,
  );
  $panes['nearest_store'] = array(
    'callback' => 'cs_uc_checkout_pane_nearest_store',
    'title' => t('Nearest store'),
    'desc' => t('Select nearest retail store.'),
    'weight' => 4,
    'process' => true,
    'collapsible' => false,
  );
  return $panes;
}


/**
 * Handler function for custom contact checkout pane.
 */
function cs_uc_checkout_pane_contact($op, $order, $main_form = NULL, &$form_state = NULL) {
  switch ($op) {
    case 'view':
      $form['billing_first_name'] = array(
        '#type' => 'textfield',
        '#title' => t('First name'),
        '#size' => 40,
        '#maxlength' => 40,
        '#required' => true,
        '#default_value' => $order->billing_first_name,
      );
      $form['billing_last_name'] = array(
        '#type' => 'textfield',
        '#title' => t('Last name'),
        '#size' => 40,
        '#maxlength' => 40,
        '#required' => true,
        '#default_value' => $order->billing_last_name,
      );
      $form['primary_email'] = array(
        '#type' => 'textfield',
        '#title' => t('E-mail'),
        '#size' => 60,
        '#maxlength' => 96, // Ubercart field length.
        '#required' => false,
        '#default_value' => $order->primary_email,
      );
      $form['billing_phone'] = array(
        '#type' => 'textfield',
        '#title' => t('Phone number'),
        '#size' => 20,
        '#maxlength' => 20,
        '#required' => false,
        '#default_value' => $order->billing_phone,
      );
      $form['legal_csa'] = array(
        '#type' => 'checkbox',
        '#title' => t('I accept terms and conditions of purchase'),
        '#required' => false,
        '#description' => t('See Cellular Sales <a href="@legal-csa" target="_blank">Legal CSA</a> for complete terms and conditions.', 
          array('@legal-csa' => url('legal/csa'))),
        '#default_value' => isset($order->data['cs_legal_csa']) ? $order->data['cs_legal_csa'] : 0,
      );
      
      $description = t('Enter your contact information here. You must enter your e-mail or phone number.');
      return array('description' => $description, 'contents' => $form);

    case 'process':
      $ok = true;
      $values = $form_state['values']['panes']['contact']; 
      $email = $values['primary_email'];
      if ($email) {
        if ($error = user_validate_mail($email)) {
          form_set_error('panes][contact][primary_email', $error);
          $ok = false;
        }
      }
      $phone = $values['billing_phone'];
      if ($phone && preg_match_all('/[0-9]/', $phone, $matches) && count($matches[0]) < 10) {
        form_set_error('panes][contact][billing_phone', t('Phone number must have at least 10 digits. Please include the area code.'));
        $ok = false;
      }
      $order->primary_email = $email;
      $order->billing_phone = $phone;
      $order->billing_first_name = $values['billing_first_name'];
      $order->billing_last_name = $values['billing_last_name'];
      $order->data['cs_legal_csa'] = $values['legal_csa'];
      if ($ok && !$email && !$phone) {
        form_set_error('', t('You must enter your e-mail address or phone number.'));
        $ok = false;
      }
      if (!$values['legal_csa']) {
        form_set_error('legal_csa', t('You must agree to the terms and conditions to complete your purchase.'));
      }
      return $ok;

    case 'review':
      $review[] = array('title' => t('Name'), 'data' => check_plain($order->billing_first_name . ' ' . $order->billing_last_name));
      if (!empty($order->primary_email)) {
        $review[] = array('title' => t('E-mail'), 'data' => check_plain($order->primary_email));
      }
      if (!empty($order->billing_phone)) {
        $review[] = array('title' => t('Phone number'), 'data' => check_plain($order->billing_phone));
      }
      return $review;
  }
}



/**
 * Handler function for nearest store checkout pane.
 */
function cs_uc_checkout_pane_nearest_store($op, $order, $main_form = NULL, &$form_state = NULL) {
  switch ($op) {
    case 'view':
      $zip = $order->billing_postal_code;
      $zip = empty($zip) ? cs_default_zip() : $zip;
      $stores = cs_zip_nearest_stores($zip, true);
      $default_store = $order->delivery_company;
      if (!$default_store) {
        $cookie = cs_location_cookie();
        if (!empty($cookie['store'])) {
          $default_store = $cookie['store']->storecode;
        }
      }
      if ($default_store && !array_key_exists($default_store, $stores)) {
        $default_store = null;
      }
      $form['store'] = array(
        '#type' => 'select',
        '#title' => t('Select store'),
        '#options' => $stores,
        '#size' => 6,
        '#description' => '',
        '#multiple' => false,
        '#required' => true,
        '#default_value' => $default_store,
        '#prefix' => '<div id="zip-stores">',
        '#suffix' => '</div>',
      );
      $form['zip'] = array(
        '#type' => 'textfield',
        '#title' => t('Zip code'),
        '#size' => 10,
        '#maxlength' => 12,
        '#required' => false,
        '#default_value' => $zip,
        '#description' => t("Enter a new zip code and click on the list above if don't see a nearby store."),
        '#ajax' => array('callback' => 'cs_uc_checkout_pane_nearest_store_update', 'wrapper' => 'zip-stores'),
      );
      
      return array('description' => t('Select the nearest store to pick up your order.'), 'contents' => $form);

    case 'process':
      $values = $form_state['values']['panes']['nearest_store'];
      $order->billing_postal_code = $values['zip']; // One they entered.
      
      $store = cs_store_by_code($values['store']);
      if (!$store) {
        return false;
      } else {
        $order->delivery_company = $store->storecode;
        $order->delivery_street1 = $store->name;
        $order->delivery_street2 = cs_format_store_address($store, false, false);
        return true;
      }

    case 'review':
      return array('data' => check_plain($order->delivery_street1 . ': '. $order->delivery_street2));
  }
}


/**
 * Ajax handler to update the nearest store based on zip code change.
 */
function cs_uc_checkout_pane_nearest_store_update($form, $form_state) {
  return $form['panes']['nearest_store']['store'];
}


/**
 * Implements hook_uc_invoice_templates() for customized versions of UC email templates.
 * 
 * Customized templates are stored in the theme templates folder.
 */
function cs_uc_invoice_templates() {
  return array('cs_admin', 'cs_customer');
}


/**
 * Called by the theme template function cellsales7_preprocess_uc_order to add our store variables for the template.
 */
function cs_preprocess_uc_order(&$variables) {
  $order = &$variables['order'];
  
  $store = cs_store_by_code($order->delivery_company);
  $addr = check_plain($store->street);
  if (!empty($store->additional)) {
    $addr .= '<br />' . check_plain($store->additional);
  }
  $addr .= '<br />' . check_plain(ucwords(strtolower($store->city))) . ', ' . check_plain($store->province) . ' ' . check_plain($store->postal_code);
  if (!empty($store->phone)) {
    $addr .= '<br />' . check_plain($store->phone);
  }
  if (!empty($store->hours)) {
    $addr .= '<br />' . $store->hours; // This is already HTML with breaks embedded.
  }
  
  $url = url("store_location/$store->storecode", array('absolute' => true, 'https' => false));
  
  foreach($order->products as $key => $product) {
    if (!isset($product->rebate)) {
      $node = node_load($product->nid);
      $rebate = cs_product_rebate($node);
      if ($rebate > 0) {
        $product->rebate = '<span style="color: red;">-$' . number_format($rebate, 2) . '</span>';
        $product->price = '$' . number_format($product->price + $rebate, 2);
      } else {
        $product->rebate = '';
        $product->price = '$' . number_format($product->price);
      }
    }
  }
  
  $variables['cs_store_info'] = l(check_plain($store->name), $url) . '<br />' . $addr;
  $variables['cs_store_image'] = l(cs_store_image($store->storecode), $url, array('html' => true));
  
  global $base_url;
  $variables['base_url'] = $base_url;
  
}



/**
 * Implements hook_uc_cart_pane_alter() to add suggested links after the items but before the buttons.
 * If we did not have to add in between, it would be better to use hook_uc_cart_pane to create our own pane.
 */
function cs_uc_cart_pane_alter(&$panes, $items) {
  if ($items && isset($panes['cart_form'])) {
    $links = '';
    $types = array();
    foreach ($items as $purchase) {
      $types[$purchase->type] = true;
    }
    if (isset($types['phone'])) {
      if (!isset($types['plan'])) {
        $links = l('Add a plan', 'plans');
      }
      if (!isset($types['accessory'])) {
        if ($links) {
          $links .= '&nbsp;|&nbsp;';
        }
        // $links .= l('Add accessories', 'phones', array('query' => array('accessory' => REQUEST_TIME)));
        $links = l('Add accessories', 'accessories');
      }
    }
    if ($links) {
      $panes['cart_form']['body']['cs_cart_links'] = array(
        '#markup' => $links,
        '#prefix' => '<div id="cs-cart-links">' ,
        '#suffix' => '</div>',
        '#weight' => 10);
    }
  }
}


/**
 * Implements hook_form_views_exposed_form_alter().
 * 
 * This works around bugs in the views pager where an illegal choices message appears.
 * Also provides support to jump to the view with device options checked.
 */
function cs_form_views_exposed_form_alter(&$form, &$form_state, $form_id) {
  switch ($form_state['view']->name) {
    case 'phones':
    case 'accessories':

      // This is a hack to get into the view with specific items checked. Used for suggested cart links.
      $query = drupal_get_query_parameters();
      $ajax = isset($_SERVER['HTTP_X_REQUESTED_WITH']) ? $_SERVER['HTTP_X_REQUESTED_WITH'] == 'XMLHttpRequest' : false;
      if ($query && !$ajax) {  // From a link we passed, not the ajax post.
        $device = array(1 => 'phone', 2 => 'tablet', 3 => 'accessory', 4 => 'plan', 5 => 'insurance', 6 => 'internet_device', 7 => 'home_solution', 8 => 'basic_phone');
        foreach ($device as $key => $value) {
          if (isset($query[$value])) {
            $session = 'cs_phones_' . $value;
            if (!isset($_SESSION[$session]) || $_SESSION[$session] != $query[$value]) {
              $_SESSION[$session] = $query[$value]; // Track the value so we only alter the form once.
              $form_state['input']['field_product_type_tid'][$key] = $key;
            }
          }
        }
      }
      
      // $check = array('field_product_type_tid', 'field_manufacturer_value', 'sell_price');
      $check = array('sell_price'); // Now only needed for sell_price. Stopped using grouped filters on the others.
      foreach ($check as $filter) {
        // Another grouped filter bug: They post 0s for empty items which gives errors and does not work.
        if (isset($form_state['input'][$filter])) {
          foreach ($form_state['input'][$filter] as $key => $item) {
            if ($item == 0) {
              unset($form_state['input'][$filter][$key]);
            }
          }
        }
      }
      
      break;
  }
}



/**
 * Implements hook_views_query_alter().
 * 
 * Exposed group filters are stupid. There is no option to OR the filters instead of ANDing them.
 * Also, the filters don't use the right values with taxonomy or fields with enumerated values.
 * The phones view could almost be fixed by using non-grouped filters and the BEF module,
 * but we still need the grouped filter for the prices ranges and that uses AND instead of OR.
 *
 * Finally able to get product type and manufacturer rendered as checkboxes without the grouped option.
 * So the main purpose of the function below is to convert the grouped option for sell_price to and OR query.
 */
function cs_views_query_alter(&$view, &$query) {  
  switch ($view->name) {
    case 'phones':
    case 'accessories':
      $match = array();
      $values = array();
      $watch = array(
        // 'field_product_type_tid' => 'field_data_field_product_type_value_0.field_product_type_tid',
        // 'field_manufacturer_value' => 'field_data_field_manufacturer_value_0.field_manufacturer_value',
        'sell_price' => 'uc_products.sell_price'
      );
      
      // The extra we are removing is an invalid AND clause they add to the join which overrides the user choices.
      if (isset($query->table_queue['field_data_field_product_type_value_0']['join']->extra)) {
        $query->table_queue['field_data_field_product_type_value_0']['join']->extra = array();
      }
      foreach ($query->where[1]['conditions'] as $index => $cond) {
        $field = $cond['field'];
        if (is_object($field)) { // Condition for the filter for taxonomy and field value queries.
          $conditions = $field->conditions(); 
          $cond = $conditions[0];
          $field = $cond['field'];
        }
        if ($key = array_search($field, $watch)) {
          if (!isset($match[$key])) {
            $match[$key] = db_or();
            
            $values[$key] = isset($view->exposed_input[$key]) ? $view->exposed_input[$key] : array();
            if ($values[$key]) {
              $filter = $view->filter[$key];
              $options = array();
              switch ($key) {
                case 'field_product_type_tid':
                  $options = array_values($filter->options['value']); // Array with correct taxonomy ID. Corresponds to ordinal value of checkboxes starting at 1.
                  break;
                case 'field_manufacturer_value':
                  $options = array_values($filter->value_options); // Keys and values are the same.
                  break;
                default:
                  $values[$key] = array(); // Last one actually works. Amazing.
              }
              if ($options) {
                foreach ($values[$key] as $k => $v) {
                    $values[$key][$k] = $options[$k-1]; // Checkboxes ordinal starting at 1.
                  }
              }
            }
          }
          if ($values[$key]) {
            $cond['value'] = array_pop($values[$key]);
          }
          $match[$key]->condition($watch[$key], $cond['value'], $cond['operator']);
          unset($query->where[1]['conditions'][$index]);
        }
      }
      $index = 100; // Use numeric indexes for add_where so user filter conditions are added at the end of the where clause.
      foreach ($match as $key => $or) {
        $query->add_where($index, $or);
        $index++;
      }
      break;
  }
}
