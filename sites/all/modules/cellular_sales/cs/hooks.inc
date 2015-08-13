<?php
define('SMARTPHONE_TERM_ID', 1);
define('CONTRACT_PRICING','contract');
define('EDGE_PRICING','edge');
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
    'cs_monthly_price' => array(
      'variables' => array('monthly_price' => 0),
    ),
    'cs_due_today_price' => array(
      'variables' => array('price' => 0),
    ),
    'cs_simple_number' => array(
      'variables' => array('qty' => 0),
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
    return 'N/A';
  }
}

/**
 * Theme for cs monthly price, similar to theme_uc_price.
 */
function theme_cs_monthly_price($variables) {
  if (isset($variables['monthly_price']) &&  $variables['monthly_price'] > 0) {
    return '<span class="cs-monthly-price">$' . number_format($variables['monthly_price'], 2) . '</span>';
  } else {
    return 'N/A';
  }
}
/**
 * Theme for cs due today price, similar to theme_uc_price.
 */
function theme_cs_due_today_price($variables) {
  $price = $variables['price'];
  if (isset($variables['price']) &&  $variables['price'] > 0) {
    $price = number_format($variables['price'], 2);    
  } 
  return '<span class="cs-due-today">$' . $price . '</span>';
}
/**
 * Theme for cs quantity, show quantity without multiplication symbol.
 */
function theme_cs_simple_number($variables) {
  if (isset($variables['qty']) &&  $variables['qty'] > 0) {
    return '<span class="cs-qty">' . $variables['qty'] . '</span>';
  } else {
    return 'N/A';
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
 * Return the product unit price for the node, if any.
 * If there is edge pricing then Full Retail Price will be returned
 */
function cs_product_unit_price($node) {  
  if ($node) {
    if($node->price_plan==EDGE_PRICING){
      return cs_product_full_retail_price(cs_product_monthly_price($node));
    }else{
      $unit_price = isset($node->list_price) ? $node->list_price : $node->sell_price;
      return round($unit_price, 2);
    }
  }  
}
/**
 * Return the product full retail price for the node, if any.
 */
function cs_product_full_retail_price($monthly_price) {
  return round($monthly_price * 24,2); //Full retail price
}
/**
 * Return the product monthly price for the node, if any.
 */
function cs_product_monthly_price($node) {
  if ($node->price_plan==EDGE_PRICING && !empty($node->field_edge_price['und'][0]['value'])) {
    return $node->field_edge_price['und'][0]['value'];
  }  else {
    return 0;
  }
}

/**
 * Return the product due today price for the node, if any.
 */
function cs_product_due_today_price($node) {
  $due_today = 0;
  if($node->price_plan!=EDGE_PRICING){
    $due_today = round(cs_product_unit_price($node)-cs_product_rebate($node),2);
  }
  return $due_today;
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
      $node = $build['#entity'];
      $node->price_plan = _get_product_price_plan($node);
      $rebate = ($node->price_plan!=EDGE_PRICING) ? cs_product_rebate($node) : 0;
      $build['rebate'] = array('#theme' => 'cs_rebate', '#rebate' => $rebate, '#cell_attributes' => array('class' => array('rebate')));
      $build['price']['#price'] = cs_product_unit_price($node);
      // adding monthly price field
      $build['monthly_price'] = array('#theme' => 'cs_monthly_price', '#monthly_price' => cs_product_monthly_price($node), '#cell_attributes' => array('class' => array('monthly-price')));
      // changing total field to due today (multiplying due today with quantity)
      $build['total']['#price'] = cs_product_due_today_price($node) * $build['qty']['#qty'];
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
        '#size' => 50,
        '#maxlength' => 40,
        '#required' => true,
//        '#default_value' => $order->billing_first_name,
        '#attributes' => array('placeholder' =>array(t('Enter First Name'))),
        '#prefix'=>'<div class="contact-info-form"><div class = "first-last-name-div">',
      );
      $form['billing_last_name'] = array(
        '#type' => 'textfield',
        '#title' => t('Last name'),
        '#size' => 50,
        '#maxlength' => 40,
        '#required' => true,
//        '#default_value' => $order->billing_last_name,
        '#attributes' => array('placeholder' =>array(t('Enter Last Name'))),
        '#suffix' => '</div>'
      );
      $form['primary_email'] = array(
        '#type' => 'textfield',
        '#title' => t('E-mail'),
        '#size' => 50,
        '#maxlength' => 96, // Ubercart field length.
        '#required' => false,
//        '#default_value' => $order->primary_email,
        '#attributes' => array('placeholder' =>array(t('Enter Email Address'))),
        '#prefix'=>'<div class = "email-phone-div">',
      );
      $form['billing_phone'] = array(
        '#type' => 'textfield',
        '#title' => t('Phone number'),
        '#size' => 50,
        '#maxlength' => 20,
        '#required' => true,
//        '#default_value' => $order->billing_phone,
        '#attributes' => array('placeholder' =>array(t('Enter Phone Number'))),
        '#suffix' => '</div></div>'
      );
      $form['legal_csa'] = array(
        '#type' => 'checkbox',
        '#title' => t('I accept terms and conditions of purchase'),
        '#required' => false,
        '#description' => t('See Cellular Sales <a href="@legal-csa" target="_blank">Legal CSA</a> for complete terms and conditions.', 
          array('@legal-csa' => url('legal/csa'))),
//        '#default_value' => isset($order->data['cs_legal_csa']) ? $order->data['cs_legal_csa'] : 0,
      );
      $description = '';
//      $description = t('Enter your contact information here. You must enter your e-mail or phone number.');
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
      drupal_set_message(t('<strong>PLEASE REVIEW: </strong>Check your order and contact information before submitting your order'), 'info');
      
      $review[] = array('title' => t('Your Name'), 'data' => check_plain($order->billing_first_name . ' ' . $order->billing_last_name));
      if (!empty($order->primary_email)) {
        $review[] = array('title' => t('Email Address'), 'data' => check_plain($order->primary_email));
      }
      if (!empty($order->billing_phone)) {
        $review[] = array('title' => t('Phone Number'), 'data' => check_plain($order->billing_phone));
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
        '#required' => true,
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
        return array(array('title' => t('Nearest Store'), 'data' => check_plain($order->delivery_street1 . ': '. $order->delivery_street2)));      
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
/**
 * below function is commented because we don't want accessories link on cart page any more
 */
function cs_uc_cart_pane_alter(&$panes, $items) {
//  dev_print_r($panes);
//  if ($items && isset($panes['cart_form'])) {
//    $links = '';
//    $types = array();
//    foreach ($items as $purchase) {
//      $types[$purchase->type] = true;
//    }
//    if (isset($types['phone'])) {
//      if (!isset($types['plan'])) {
//        $links = l('Add a plan', 'plans');
//      }
//      if (!isset($types['accessory'])) {
//        if ($links) {
//          $links .= '&nbsp;|&nbsp;';
//        }
//        // $links .= l('Add accessories', 'phones', array('query' => array('accessory' => REQUEST_TIME)));
//        $links = l('Add accessories', 'accessories');
//      }
//    }
//    if ($links) {
//      $panes['cart_form']['body']['cs_cart_links'] = array(
//        '#markup' => $links,
//        '#prefix' => '<div id="cs-cart-links">' ,
//        '#suffix' => '</div>',
//        '#weight' => 10);
//    }
//  }
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

/**
 * Code by TM
 */

function dev_print_r($str){
    echo '<pre>';
    print_r($str);
    echo '</pre>';    
}
function dev_print_exit($str){
    echo '<pre>';
    print_r($str);
    echo '</pre>';    
    exit;
}

/**
 * Overriding the view cart form in order to add new fields in it 
 * @param type $form
 * @param type $form_state
 */
function cs_form_uc_cart_view_form_alter(&$form, &$form_state) { 
  $form['#attached']['css']=array(libraries_get_path('uniform') . "/theme/css/uniform.default.css");
  $form['#attached']['js']=array(libraries_get_path('uniform') . "/jquery.uniform.js",
                                  drupal_get_path('module', 'cs') . '/cart.js');
  // changing title and weight of already added columns
  $form['items']['#columns']['image']['cell'] = t('Item');
  $form['items']['#columns']['remove']['cell'] = '';
  $form['items']['#columns']['remove']['weight'] = 3;
  $form['items']['#columns']['qty']['weight'] = 3;
  // adding new columns
  $form['items']['#columns']['unit_price'] = array(
      'cell' => t('Unit Price'), 
      'weight' => 4,
  );
  $form['items']['#columns']['monthly_price'] = array(
      'cell' => t('Monthly Price'), 
      'weight' => 5,
  );
  // changing column total to due today
  $form['items']['#columns']['total'] = array(
      'cell' => t('Due Today'), 
      'weight' => 6,
  );
//  Adding classes and divs in order to style them
  $form['actions']['continue_shopping']['#suffix']='<div class= "update-checkout-btn-div">';
  $form['actions']['checkout']['checkout']['#suffix']='</div>';
  $form['actions']['checkout']['checkout']['#attributes']=array('class'=>array('checkout-cart'));
}

/**
 * Overriding the tapir table in order to change the view of view cart page
 * @param type $table
 * @param type $table_id
 */
function cs_tapir_table_alter(&$table, $table_id) {
  if ($table_id == 'uc_cart_view_table') {
    
    $subtotal_due_today = 0;
    foreach (element_children($table) as $key) {
      
      if (!empty($table['#parameters'][1][$key]['nid'])) {
        
        // Adding values to new added fields
        $node = $table['#parameters'][1][$key]['#entity'];
        $node->price_plan = _get_product_price_plan($node);
        
        $table[$key]['desc']['#cell_attributes'] = array('class' => array('products'));
        
        // changing quantity field from text field to dropdown
        $table[$key]['qty']['#type'] = 'select';
        $table[$key]['qty']['#options'] = get_quantity_options(10);
        
        // change image style to enlarge its view
        $table[$key]['image'] = uc_product_get_picture($node->nid, 'cs_cart_product');
                        
        $table[$key]['unit_price'] = array(
          '#markup' => theme('uc_product_price',array('element' => array('#value' => cs_product_unit_price($node)))),
        );
        $table[$key]['monthly_price'] = array(
          '#markup' => theme('cs_monthly_price', array('monthly_price' => cs_product_monthly_price($node))),
        );
        
        // changing total field to due today
        $table[$key]['total']['#theme'] = 'cs_due_today_price';
        $table[$key]['total']['#cell_attributes'] = array('class' => array('due-today'));
        $table[$key]['total']['#price']= cs_product_due_today_price($node) * $table[$key]['qty']['#default_value'];
        
        // adding due today to calculate total due today
        $subtotal_due_today+= $table[$key]['total']['#price'];
      }
    }
    // find subtotal column and modify it
    if(!isset($table[$key]['qty']) && !isset($table[$key]['remove'])){        
      $table[$key]['total']['#prefix'] = '<span id="subtotal-title">' . t('Subtotal due today') . '</span> ';
      $table[$key]['total']['#price'] = $subtotal_due_today;
    }
  }
}

function cs_form_uc_cart_checkout_review_form_alter(&$form){
  $form['actions']['back']['#value']=t('Edit Info');
  $form['actions']['submit']['#value']=t('Place Order');
}

function cs_form_uc_cart_checkout_form_alter(&$form, &$form_state){
  $form['#attached']['css']=array();
}
/**
 * IMplementation of hook_form_alter for adding smartphones validation
 * @param type $form
 * @param type $form_state
 */
function cs_form_phone_node_form_alter(&$form){
  $form['base']['prices']['sell_price']['#required']=FALSE;
  $form['#validate'][] = 'cs_phone_node_validate';
}
/**
 * Validate method for smart phones
 * it validates that if a product is of type smart phone then edge pricing is mandatory
 * @param type $form
 * @param type $form_state
 */
function cs_phone_node_validate(&$form,&$form_state){
  $phone = $form_state['values'];
  if($phone['field_product_type']['und'][0]['tid'] == SMARTPHONE_TERM_ID && empty($phone['field_edge_price']['und'][0]['value'])){
    form_set_error('field_edge_price', t('Edge Price is required for product type <strong>SmartPhone</strong>'));
  }
}
/**
 * get array of options to show in quantity dropdown
 * array(1=>'1',2=>'2',3=>'3',4=>'4',5=>'5',6=>'6',7=>'7',8=>'8',9=>'9',10=>'10');
 * @param type $upto
 * @return int
 */
function get_quantity_options($upto){
  
  $options = array();
  $index=1;
  while($index<=$upto){
    $options[$index]=$index;
    $index++;
  }
  return $options;
}

/**
 * IMplementation of hook_form_alter for adding element on add to cart form
 * @param type $form
 * @param type $form_state
 */
function cs_form_alter(&$form, &$form_state,$form_id) {
  if (strstr($form_id, 'uc_product_add_to_cart_form_')) {
    
    $default_price_plan = CONTRACT_PRICING;
    if(!empty($form['node']['#value']->field_edge_price['und'][0]['value'])){
      $default_price_plan = EDGE_PRICING;
    }
    $form['#attached']['js'][] = drupal_get_path('module', 'cs') . '/cart.js';
    $form['price_plan'] = array('#type' => 'hidden', '#default_value' => $default_price_plan);
    $form['#submit'][] = 'cs_add_to_cart_price_submit';
  }
}

/**
 * Validate method for smart phones
 * it validates that if a product is of type smart phone then edge pricing is mandatory
 * @param type $form
 * @param type $form_state
 */
function cs_add_to_cart_price_submit($form,&$form_state){  
  $ob = $form_state['values'];  
  $cart_id = uc_cart_get_id();
  $price_plan = $ob['price_plan'];
  $nid = $ob['node']->nid;
  if(_is_product_price_plan_exists($cart_id, $nid)){
    _update_product_price_plan($cart_id,$nid,$price_plan);
  }else{
    _add_product_price_plan($cart_id,$nid,$price_plan);
  }
}
/**
 * Add the price plan of a product
 * @param type $cart_id
 * @param type $nid
 * @param type $price_plan
 */
function _add_product_price_plan($cart_id,$nid,$price_plan){
  db_insert('cs_cart_product_price_plan')
    ->fields(array(
      'cart_id' => $cart_id,
      'nid' => $nid,
      'price_plan' => $price_plan
    ))
    ->execute();
}
/**
 * Update the price plan of a product 
 * @param type $cart_id
 * @param type $nid
 * @param type $price_plan
 */
function _update_product_price_plan($cart_id,$nid,$price_plan){
  db_update('cs_cart_product_price_plan')
    ->fields(array(
      'price_plan' => $price_plan
    ))
    ->condition('cart_id', $cart_id)
    ->condition('nid', $nid)
    ->execute();
}
/**
 * Check if product price plan already exist
 * @param type $cart_id
 * @param type $nid
 * @return type
 */
function _is_product_price_plan_exists($cart_id, $nid) {
  $query = db_select('cs_cart_product_price_plan', 'cpp')
    ->fields('cpp', array('id'))
    ->condition('cart_id', $cart_id)
    ->condition('nid', $nid)
    ->execute()
    ->fetchField();
  return empty($query) ? FALSE : TRUE;
}

function _get_product_price_plan($node) {
  $cart_id = isset($node->cart_id)?$node->cart_id:uc_cart_get_id();
  $plan = db_select('cs_cart_product_price_plan', 'cpp')
    ->fields('cpp', array('price_plan'))
    ->condition('cart_id', $cart_id)
    ->condition('nid', $node->nid)
    ->execute()
    ->fetchField();
  if (empty($plan)){
    return !empty($node->field_edge_price['und'][0]['value'])? EDGE_PRICING:CONTRACT_PRICING; 
  }
  return $plan;
}
