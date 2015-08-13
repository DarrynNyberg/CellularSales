<?php

/**
 * @file
 * Preorder page and form.
 */


/**
 * Main preorder page.
 */
function storeloc_preorder_page() {
  if (!isset($_SERVER['HTTPS']) || $_SERVER['HTTPS'] !== 'on') {
    if (stripos($_SERVER['HTTP_HOST'], 'cellularsales.com') !== false) {
      global $base_url;
      $base = str_replace('http://', 'https://', $base_url);
      drupal_goto($base . request_uri());
    }
  }
  $id = empty($_GET['id']) ? '' : $_GET['id'];
  return drupal_get_form('storeloc_preorder_form', null, $id);
}


/**
 * Ajax callback handler for adding phone lines and updating stores based on zip.
 */
function storeloc_preorder_ajax($op = '') {
	$element = null;
	$form = cs_ajax_callback_helper($op);
	switch ($op) {
		case 'store':
			$element = $form['loc']['store'];
			break;
		
		case 'add_phone':
		case 'remove_phone':
			$element = $form['new']['phones'];
			break;	
	}
	
	if ($element) {
		$status = true;
		unset($element['#prefix'], $store_select['#suffix']); // Prevent duplicate wrappers.
		$output = theme('status_messages') . drupal_render($element);
	} else {
		$status = false;
	}
  drupal_json(array('status' => $status, 'data' => $output));
}



function storeloc_preorder_form(&$form_state, $id = '') {
  $form['#id'] = $id; // ID embedded in the emailed URL.
	$ajax = !empty($form_state['ajax_submission']) ? $form_state['ajax_submission'] : '';
  
  $form[] = array(
    '#type' => 'markup',
    '#value' => '<p>This pre-order form is for existing Verizon Wireless customers. 
    If you would like to pre-order and you do not have an existing Verizon Wireless account, 
    please call 877-977-1244 and speak to a live wireless expert to place your pre-order.</p><br>

    <p>If you have any questions about your account or would like to speak to a live customer support specialist, call 877-977-1244.</p>',
  );
  
  $form['first_name'] = array(
    '#type' => 'textfield',
    '#title' => t('First name'),
    '#size' => 40,
    '#maxlength' => 40,
    '#required' => true,
  );
  $form['last_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Last name'),
    '#size' => 40,
    '#maxlength' => 40,
    '#required' => true,
  );
  $form['email'] = array(
    '#type' => 'textfield',
    '#title' => t('Email'),
    '#size' => 80,
    '#maxlength' => 254,
    '#required' => true,
  );
  $form['email_verify'] = array(
    '#type' => 'textfield',
    '#title' => t('Verify email'),
    '#size' => 80,
    '#maxlength' => 254,
    '#required' => true,
    '#description' => t('Please retype your email to verify.')
  );
  $form['vw_phone'] = array(
    '#type' => 'textfield',
    '#title' => t('Verizon Wireless mobile number'),
    '#size' => 20,
    '#maxlength' => 20,
    '#required' => true,
  );
  $form['ssn4'] = array(
    '#type' => 'textfield',
    '#title' => t('Last 4 SSN'),
    '#size' => 6,
    '#maxlength' => 4,
    '#required' => true,
    '#description' => t('Last four digits of your social security number.')
  );
  $form['vw_password'] = array(
    '#type' => 'password',
    '#title' => t('Verizon Wireless password'),
    '#size' => 20,
    '#maxlength' => 20,
    '#required' => false,
    '#description' => t('Your Verizon Wireless account password, if applicable.'),
  );
  $form['primary_phone'] = array(
    '#type' => 'textfield',
    '#title' => t('Primary contact number'),
    '#size' => 20,
    '#maxlength' => 20,
    '#required' => true,
    '#description' => t('Number to call if we need to contact you about your pre-order.'),
  );
  
  $form['new'] = array(
    '#type' => 'fieldset',
    '#title' => t('Pre-Order Selection'),
  );
	$form['new']['phones'] = array(
		'#type' => 'markup',
		'#prefix' => '<div id="new-phone-list">',
		'#suffix' => '</div>',
	);
	
	$phone_count = empty($form_state['values']['phone_count']) ? 1 : $form_state['values']['phone_count'];
	if ($ajax == 'add_phone') {
		$phone_count++;
		if ($phone_count > 10) {
			$phone_count = 10;
		}
	} else if ($ajax == 'remove_phone' && $phone_count > 1) {
		$phone_count--;
	}
	
	
	// Note we could have just used $form_state rather than a hidden field, but went to this in trying to debug other issues.
	$form['phone_count'] = array(
		'#type' => 'hidden',
		'#value' => $phone_count,
	);

	for($i = 1; $i <= $phone_count; $i++) {
		if ($i > 1) {
			$form['new']['phones'][] = array('#type' => 'markup', '#value' => '<hr>');
		}
		
		$form['new']['phones']["add_$i"] = array(
			'#type' => 'radios',
			'#required' => true,
			'#options' => array(t('Upgrade current line'), t('Add a new line')),
			'#default_value' => isset($form_state['values']["add_$i"]) ? $form_state['values']["add_$i"] : 0,
		);
		$form['new']['phones']["mobile_phone_$i"] = array(
			'#type' => 'textfield',
			'#title' => t('Mobile number'),
			'#size' => 20,
			'#maxlength' => 20,
			'#default_value' => isset($form_state['values']["mobile_phone_$i"]) ? $form_state['values']["mobile_phone_$i"] : '',
			'#description' => t('If you are upgrading, enter the mobile number you are transferring to your new iPhone.'),
		);
		$form['new']['phones']["phone_choice_$i"] = array(
			'#type' => 'select',
			'#title' => t('Select your iPhone'),
			'#options' => array(
			  'iphone red' => 'iPhone - Red', 
			  'iphone white' => 'iPhone - White', 
			  'iphone blue' => 'iPhone - Blue',
			),
			'#multiple' => false,
			'#required' => true,
			'#default_value' => isset($form_state['values']["phone_choice_$i"]) ? $form_state['values']["phone_choice_$i"] : null,
		);
	}
	
	$form['new']['add_phone'] = array(
		'#type' => 'button',
		'#value' => t('Add another phone'),
		'#description' => t('Click here if you want to order additional phones.'),
		'#ahah' => array('path' => 'preorder_ajax/add_phone', 'wrapper' => 'new-phone-list'),
	);
	
	$form['new']['remove_phone'] = array(
		'#type' => 'button',
		'#value' => t('Remove last phone'),
		'#description' => t('Click here to remove the last phone from your order.'),
		'#ahah' => array('path' => 'preorder_ajax/remove_phone', 'wrapper' => 'new-phone-list'),
	);

	$form['loc'] = array(
    '#type' => 'fieldset',
    '#title' => t('Where do you want to pick up your phone'),
  );
  
  $zip = isset($form_state['values']['zip']) ? $form_state['values']['zip'] : cs_default_zip();
  $stores = cs_zip_nearest_stores($zip, true);
	$form['loc']['zip'] = array(
    '#type' => 'textfield',
    '#title' => t('Zip code'),
    '#size' => 10,
    '#maxlength' => 12,
    '#required' => true,
    '#default_value' => $zip,
    '#description' => t('Enter your zip code and click on the store list to select the nearest store.'),
    '#ahah' => array('path' => 'preorder_ajax/store', 'wrapper' => 'edit-store-wrapper'),
  );
  $default_store = isset($form_state['values']['store']) ? $form_state['values']['store'] : null;
  if ($default_store && !array_key_exists($default_store, $stores)) {
    $default_store = null;
  }
  $form['loc']['store'] = array(
		'#type' => 'select',
		'#title' => t('Select store'),
		'#options' => $stores,
		'#size' => 5,
		'#description' => '',
		'#multiple' => false,
		'#required' => true,
		'#default_value' => $default_store,
	);
	
	$form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );
  
  // Without this submit button post goes to the last ajax URL!
  $form['#action'] = '/preorder';
  
	return $form;
}


function storeloc_preorder_form_validate($form, &$form_state) {
	if (!empty($form_state['ajax_submission'])) {
		return;
	}
	$values = $form_state['values'];
	$email = $values['email'];
	if ($email !== '' && !valid_email_address($email)) {
      form_set_error('email', t('%email is not a valid email address.', array('%email' => $email)));
    }
	if ($email !== $values['email_verify']) {
		form_set_error('email_verify', t('Verify email must match Email field.'));
	}
	$ssn = $values['ssn4'];
	if (strlen($ssn) != 4 || !ctype_digit($ssn)) {
	  form_set_error('ssn4', t('Last 4 SSN must be exactly four digits.'));
	} else if (array_search($ssn, array('0000')) !== false) {
	  form_set_error('ssn4', t('Invalid value for Last 4 SSN. Please call 877-977-1244.'));
	}
	
	$phones = array('vw_phone', 'primary_phone');
	for ($i = 1 ; $i <= $values['phone_count']; $i++) {
	  if ($values["add_$i"] != 1) {
	    $key = "mobile_phone_$i";
	    if ($values[$key]) {
	      $phones[] = $key;
	    } else {
	      form_set_error($key, t('Mobile number is required if you are upgrading.'));
	    }
	  }
	}
	foreach ($phones as $key) {
    $matches = array();
    if ($values[$key] && preg_match_all('/[0-9]/', $values[$key], $matches) && count($matches[0]) < 10) {
      form_set_error($key, t('Phone numbers must have at least 10 digits. Please include the area code.'));
    }
  }
	
}

function storeloc_preorder_form_submit($form, &$form_state) {
	if (!empty($form_state['ajax_submission'])) {
		return;
	}
	$stamp = date('c');
	$data = serialize($form_state['values']);
	$res = db_query("INSERT INTO {storeloc_preorder} (stamp, data) VALUES ('%s', '%s')", array($stamp, $data));
	if ($res === false) {
	  watchdog('storeloc', $data, null, WATCHDOG_ERROR); // Try to store it this way, just in case.
	}
	
	drupal_set_message(t("Thank-you for choosing Cellular Sales as your wireless experts! We'll contact you when your phone has arrived for pick-up!"));
	// $form_state['redirect'] = 'store_location/' . $form_state['values']['store'];
	global $base_url;
	$form_state['redirect'] = str_replace('https://', 'http://', $base_url) . '/store_location/' . $form_state['values']['store'];
}


/**
 * Called by cron to export preorder data to be picked up by CS.
 */
function storeloc_preorder_export() {
  $file = 'CallCenterIconicPreOrder_' . date('YmdHis') . '.txt';
  $path = file_directory_path() . '/preorder/' . $file;
  $temp = file_directory_temp() . '/' . $file;
  $last = variable_get('storeloc_preorder_last_export', 0);
  $count = 0;
  $res = db_query("SELECT id, data FROM {storeloc_preorder} WHERE id > %d ORDER BY id", array($last));
  if ($res && $handle = fopen($temp, 'w')) {
    while ($r = db_fetch_object($res)) {
      $count++;
      $last = $r->id;
      $values = unserialize($r->data);
      
      // Change all mobile numbers to remove anything but digits.
      $values['vw_phone'] = preg_replace('/[^0-9]/', '', $values['vw_phone']);
      
      $info = "{$values['first_name']} {$values['last_name']}/n";
      $info .= "Primary Contact #: {$values['primary_phone']}/n";
      $info .= "Email: {$values['email']}/n";
      $info .= "Preferred Store: {$values['store']}/n";
      
      for ($i = 1 ; $i <= $values['phone_count']; $i++) {
        $phone = $values["phone_choice_$i"];
    	  if ($values["add_$i"] != 1) {
    	    $key = "mobile_phone_$i";
    	    $values[$key] = preg_replace('/[^0-9]/', '', $values[$key]);
    	    $info .= $values[$key] . ': ' . $phone . "/n";
    	  } else {
    	    $info .= "New: {$phone}/n";
    	  }
    	}
    	$info = str_replace('|', ' ', $info);
    	$values['vw_password'] = str_replace('|', ' ', $values['vw_password']); // Not much we can do...
    	
    	$line = "{$r->id}|{$values['ssn4']}|{$values['vw_phone']}|{$values['vw_password']}|{$info}\n";
    	fwrite($handle, $line);
    }
    fclose($handle);
    if ($count > 0 && rename($temp, $path)) {
      variable_set('storeloc_preorder_last_export', $last);
    }
    if (file_exists($temp)) {
      unlink($temp);
    }
  }
}
