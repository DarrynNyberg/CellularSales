<?php 
//Customized drupal form 

//Davids final layout 8/15/14 9:50am
 /*if (!empty($field_profile_image_link[0]['value'])){
		echo "<img src =\"$linkpic\" class=\"profilepic\">"; 
	 }elseif (!empty($field_photo1[0]['filepath'])) {
     print "<img src=\"$base_url/$uploadpath\" class=\"profilepic\"/>";
	 } */
 
//Phone Ubercart variables     
$image1 = $uc_product_image[0]['filename']  ; 
//$image2 =  $field_product_image_2[0]['filename']  ;
//$image3 =  $field_product_image_3[0]['filename']  ;
//$body = $body[0]['safe_value']  ; 
//$additional_description = $field_additional_description[0]['safe_value']  ; 
//$body3 = $field_additional_description_bod[0]['safe_value']  ; 
//$specifications = $field_specifications[0]['safe_value']  ; 

$free_product = ($display_price == 0.00 && $list_price>0 && $rebate>0 && $list_price-$rebate == 0)?TRUE:FALSE;

//'module_name' = 'cs'
//'block_delta' = 'store_locator' (larger version with form) or 'nearest_store' (smaller version, no form).
//D7
$block = module_invoke('cs', 'block_view', 'nearest_store');
?>
<?php if($page) { ?>



<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title><?php print $title ;?></title>

<style>
.description { 
	width: 100%;
	padding-bottom: 50px;
	font-family: 'VerizonApexBook',Helvetica,sans-serif;
    font-size: 1em;}

.clear {
    clear:both;
}
.big_box {
    width:100%;
}
.big_box .box {
    float:left;
    width:100%;
	vertical-align: top;
}

.box {
	vertical-align: top;
	padding-bottom: 5px; 
}
h3 {
	color:#D62632;
	margin: 0;
}
.img1 {padding: 6px;}

.pricing2 {color: #FFF; }
</style>

</head>

<body>


<?php //print print_r($node) ; 

?>

<!-- Davids code -->

<div class="region region-content">
    <section class="block block-system clearfix" id="block-system-main">
        <div class="ds-1col node node-internet_device view-mode-full clearfix">
            <div class="field field-name-uc-product-image field-type-image field-label-hidden field-items field-item even"><img src="/sites/default/files/product/image/internet_device/<?php print $image1 ;?>" /></div>
            
              <div class="field-description-section-wrapper">
            
                <div class="field field-name-title field-type-ds field-label-hidden field-items field-item even">
                    <h2><?php print $title ;?></h2>
                </div>
    
                <div class="group-pricingac field-group-div" id="node_accessory_full_group_pricingac">
                    <?php if ($display_price != 0.00 || $free_product) {?>
                    <div class="product-info sell-price">
                        <span class="uc-price-label">Price:</span> <span class="uc-price">
                        <?php if ($free_product) {
                          print "FREE" ;
                        } else {
                          $parts = explode('.', $display_price);    
                          print '<span class="text-14">$</span>' . $parts[0] . '<span class="text-14">' . $parts[1] . '</span>';
                        } 
                        ?>    
                        </span><br />
                        <span class="pricing2">
                            
                            <?php  if ($rebate){
                                print "$".$list_price ." 2-yr contract price -$" . $rebate . " mail-in rebate debit card.";
                                }else{
                                  print "2-year contract price";
                                } 
                            ?>
                        
                        </span>
                    </div>
                    <?php } ?>
                    <div class="add-to-cart">
    					          <?php
                          $form_value =  drupal_get_form('uc_product_add_to_cart_form_'.$node->nid, $node);
                          print drupal_render($form_value);
                        ?>
                    </div>
                </div>  
             
             </div>
               
             <div class="field-name-nearest-store-block-wrapper">
                
                  <h4>Come in or call:</h4>
                
                  <div class="field field-name-nearest-store-block field-type-ds field-label-hidden field-items field-item even">
                    <div class="cs-store-address" id="cs-nearest-store"><?php print render($block['content']); ?></div>
                  </div>
                
                <h6>OR</h6>
                <h4>Call to set a no-wait appointment:</h4>
                <h4><span>877-851-6154</span></h4>
            </div>
                
            
            <fieldset class="group-highlights-features2 field-group-fieldset panel panel-default form-wrapper">
                <legend class="panel-heading"></legend>

<!-- Please do not modify below this line -->

                <div class="panel-title fieldset-legend">
                    <legend class="panel-heading">Highlights &amp; Features</legend>
                </div><legend class="panel-heading"></legend>

                <div class="panel-body">
                    <div class="field field-name-body field-type-text-with-summary field-label-hidden field-items field-item even"><?php  if (!empty($body[0]['safe_value'])){
					print $body[0]['safe_value'] ;
				  } 
				  ?></div>

                  
            </fieldset>

            <div class="group-specifications-wrap field-group-div" id="node-internet_device-full-group-specifications-wrap"> <?php  if (!empty($field_specifications[0]['safe_value'])){
					print "<h3>SPECIFICATIONS</h3>" ;
					
					print $field_specifications[0]['safe_value'] ;
				  } 
				  ?></div>
        </div>
<!-- /.block -->
</div>

</body>
</html>



<?php } elseif(!$page && $teaser) { ?>
    <?php
        $body = $node->body['und'][0]['value'];
        $body_strip = strip_tags($body);
        //$body_200 = substr($body_strip, 0, 200);
        $body_200 = substr($body_strip, 0, strpos($body_strip, ' ', 200))
    
    ?>
    <div class="col-sm-6 product-catalog-display">
        <div class="row">
            <div class="col-sm-4">
                <div class="product-cat-internet-device-img">
                    <img src="/sites/default/files/product/image/internet_device/<?php print $image1 ;?>" />
                </div>
            </div>
            <div class="col-sm-8">
                <span class="product-title">
                    <?php print $node->title; ?>
                </span>
                <p class="field-name-body"><?php print $body_200; ?></p>
                
                <br />
                <?php  if ($display_price != 0.00 || $free_product) {?>
                <div class="cat-pricing non-edge-price">                        
                      <?php if ($free_product) {
                          print "FREE" ;
                        } else {
                      $parts = explode('.', $display_price);
                          print '<span class="text-14">$</span>' . $parts[0] . '<span class="text-14">' . $parts[1] . '</span>';
                        } 
                      ?>
                      <br />
                      <span class="text-10">
                              <?php
                              if ($rebate) {
                                print "$" . $list_price . " 2-yr contract<br>price -$" . $rebate . " mail-in<br>rebate debit card.";
                              } else {
                                print "2-year contract price";
                              }
                              ?>
                      </span>                    
                </div>
                      <?php } ?>
                <div class="clearfix"></div>
                <br /><br />
                <?php print l('', 'node/' . $node->nid, array('attributes' => array('class' => array('btn', 'btn-default')))); ?>
        
                    
            </div>
        </div>
    </div>
<?php } ?>