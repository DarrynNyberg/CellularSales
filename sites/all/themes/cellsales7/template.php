<?php

define('VTHEME', base_path() . drupal_get_path('theme', 'cellsales7') . '/');


/**
 * @file
 * template.php
 */


function cellsales7_preprocess_uc_order(&$variables) {
  cs_preprocess_uc_order($variables);
}
/**
 * Overriding the hook in order to change Review order page title
 * @param type $variables
 */
function cellsales7_preprocess_page(&$variables){
  if(strstr($_SERVER['REQUEST_URI'], 'cart/checkout/review')){
    $variables['title'] = t('Order Summary');
  }
}
/**
 * Overriding the hook in order to change Review order page title
 * @param type $variables
 */
function cellsales7_preprocess_html(&$variables){
  if(strstr($_SERVER['REQUEST_URI'], 'cart/checkout/review')){
    $variables['head_title_array']['title'] = t('Order Summary');
    $variables['head_title'] = t('Order Summary') . ' | ' . $variables['head_title_array']['name'];
  }
}



/**
 * Formats the cart contents table on the checkout page.
 *
 * @param $variables
 *   An associative array containing:
 *   - show_subtotal: TRUE or FALSE indicating if you want a subtotal row
 *     displayed in the table.
 *   - items: An associative array of cart item information containing:
 *     - qty: Quantity in cart.
 *     - title: Item title.
 *     - price: Item price.
 *     - desc: Item description.
 *
 * @return
 *   The HTML output for the cart review table.
 *
 * @ingroup themeable
 */
function cellsales7_uc_cart_review_table($variables) {
  $items = $variables['items'];
  // $show_subtotal = $variables['show_subtotal'];
  $show_subtotal = true;

  $subtotal = 0;

  // Set up table header.
  $header = array(
	array('data' => t('Item'), 'class' => array('products')),
	array('data' => t('Qty'), 'class' => array('quantity')),
	array('data' => t('Unit Price'), 'class' => array('unit-price')),
    array('data' => t('Rebate'), 'class' => array('rebate')),
    array('data' => t('Monthly Price'), 'class' => array('monthly-price')),
    array('data' => t('Due Today'), 'class' => array('due-today')),
  );

  // Set up table rows.
  $display_items = uc_order_product_view_multiple($items);
  if (!empty($display_items['uc_order_product'])) {
    foreach (element_children($display_items['uc_order_product']) as $key) {
      $display_item = $display_items['uc_order_product'][$key];
      $subtotal += $display_item['total']['#price'];
      $display_item['qty']['#theme'] = 'cs_simple_number';
      $display_item['total']['#theme'] = 'cs_due_today_price';
      $rows[] = array(
        array('data' => $display_item['product'], 'class' => array('products')),
        array('data' => $display_item['qty'], 'class' => array('quantity')),
        array('data' => $display_item['price'], 'class' => array('unit-price')),
        array('data' => $display_item['rebate'], 'class' => array('rebate')),
        array('data' => $display_item['monthly_price'], 'class' => array('monthly-price')),
        array('data' => $display_item['total'], 'class' => array('due-today')),
      );
    }
  }

  // Add the total row as the final row.
  $rows[] = array(
    'data' => array(
      // One cell
      array(
        'data' => array(
          '#theme' => 'uc_price',
          '#prefix' => '<span id="total-title">' . t('Total due today') . '</span> ', // Changed to "Total". Why Subtotal?
          '#price' => $subtotal,
        ),
        // Cell attributes
        'colspan' => 6,
        'class' => array('total-due-today'),
      ),
    ),
    // Row attributes
    'class' => array('total-due-today'),
  );

  return theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('class' => array('cart-review'))));
}



/**
 * Themes the checkout review order page.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form, that by default includes
 *     the 'Back' and 'Submit order' buttons at the bottom of the review page.
 *   - panes: An associative array for each checkout pane that has information
 *     to add to the review page, keyed by the pane title:
 *     - <pane title>: The data returned for that pane or an array of returned
 *       data.
 *
 * @return
 *   A string of HTML for the page contents.
 *
 * @ingroup themeable
 */
function cellsales7_uc_cart_checkout_review($variables) {
  $panes = $variables['panes'];
  $form = $variables['form'];

  $output = '<div id="review-instructions">' . filter_xss_admin(variable_get('uc_checkout_review_instructions', uc_get_message('review_instructions'))) . '</div>';
  $cart_table_html = '';
  $output .= '<div class="order-review-div">';
  
  $output .= '<table class="table table-striped contact-store-table">';
  $table_header = '<thead><tr>';
  $table_body = '<tbody><tr>';
  foreach ($panes as $pane) {
    if(is_array($pane[0])){
      foreach ($pane as $row) {
        $table_header .= '<th>' . $row['title'] . '</th>';
        $table_body .= '<td>' . $row['data'] . '</td>';
      }
    }else{
      $cart_table_html = $pane[0];
    }
  }
  $table_header .='</tr></thead>';
  $table_body .='</tr></tbody>';
  $output .= $table_header . $table_body.'</table>';
  
  $output .= '<div class="pane-cart-div">';
  $output .= $cart_table_html;
  $output .= '</div>';

  $output .= '<div class="review-button-div">';
  $output .= drupal_render($form) ;
  $output .= '</div>';
  
  $output .= '</div>';
  return $output;
}

function home_banners_slider() {
?>
<ul class="rslides" id="slider1">
<?php
$query = new EntityFieldQuery();
$entities = $query->entityCondition('entity_type', 'node')
  ->propertyCondition('type', 'banner_slider')
  ->propertyCondition('status', 1)
  ->propertyOrderBy('created', 'DESC')
  ->execute();
$homeBanners = node_load_multiple(array_keys($entities['node'])); 
foreach($homeBanners as $homeBanner){	
$bannerImage = image_style_url('large', $homeBanner->field_banner_image['und'][0]['uri']);
$bannerImageName = $homeBanner->field_banner_image['und'][0]['filename'];
$bannerLink = $homeBanner->field_banner_link['und'][0]['value'];
?>
<li><a href = "<?php echo $bannerLink ?>"><img src="<?php echo $GLOBALS['base_url'].'/sites/default/files/'.$bannerImageName ?>"></a></li>
<?php } ?>
</ul>
<?php	
}