<?php
/**
 * @file
 * This file is the default customer invoice template for Ubercart.
 *
 * Available variables:
 * - $products: An array of product objects in the order, with the following
 *   members:
 *   - title: The product title.
 *   - model: The product SKU.
 *   - qty: The quantity ordered.
 *   - total_price: The formatted total price for the quantity ordered.
 *   - individual_price: If quantity is more than 1, the formatted product
 *     price of a single item.
 *   - details: Any extra details about the product, such as attributes.
 * - $line_items: An array of line item arrays attached to the order, each with
 *   the following keys:
 *   - line_item_id: The type of line item (subtotal, shipping, etc.).
 *   - title: The line item display title.
 *   - formatted_amount: The formatted amount of the line item.
 * - $shippable: TRUE if the order is shippable.
 *
 * Tokens: All site, store and order tokens are also available as
 * variables, such as $site_logo, $store_name and $order_first_name.
 *
 * Display options:
 * - $op: 'view', 'print', 'checkout-mail' or 'admin-mail', depending on
 *   which variant of the invoice is being rendered.
 * - $business_header: TRUE if the invoice header should be displayed.
 * - $shipping_method: TRUE if shipping information should be displayed.
 * - $help_text: TRUE if the store help message should be displayed.
 * - $email_text: TRUE if the "do not reply to this email" message should
 *   be displayed.
 * - $store_footer: TRUE if the store URL should be displayed.
 * - $thank_you_message: TRUE if the 'thank you for your order' message
 *   should be displayed.
 *
 * @see template_preprocess_uc_order()
 */
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>Customer Receipt</title>

<style>

.storeinfo a:link { color:#ffffff !important; }

</style>
</head>

<body>

<table width="100%" border="0" cellspacing="0" cellpadding="0">
  <tr>
    <td>
      <center>
      <table width="650" border="0" cellspacing="0" cellpadding="8">
      <tr>
        <td bgcolor="#cd091f"><img src="<?php print $base_url; ?>/sites/default/files/email_logo.jpg" alt="Cellularsales" width="205" height="79" align="left" /></td>
        <td bgcolor="#cd091f" style="color:#FFFFFF; font-size: 18px;" colspan="2"><font face="Arial"><b>You're Awesome, <?php print $order_first_name ;?></b></font></td>
        
      </tr>
      <tr>
        <td colspan="3" align="left"><font face="Arial">

Thank you for your order. One of our sales representatives will contact you shortly to set a no wait appointment for you or help you get the service you need.</font></td>
       
      </tr>
      <tr>
        <td valign="top" colspan="3">
          <table width="100%" border="0" cellspacing="0" cellpadding="6">
            <tr>
              <th bgcolor="#000000" style="color:#FFF" width="55%" align="left"><font face="Arial">PRODUCT</font></th>
              <th bgcolor="#000000" style="color:#FFF" width="15%" align="center"><font face="Arial">QTY</font></th>
              <th bgcolor="#000000" style="color:#FFF" width="25%" align="center"><font face="Arial">COST</font></th>
              <th bgcolor="#000000" style="color:#FFF" width="15%" align="center"><font face="Arial">REBATES</font></th>
              <th bgcolor="#000000" style="color:#FFF" width="25%" align="center"><font face="Arial">TOTAL</font></th>
              
            </tr>
            <?php foreach ($products as $product): ?>
            <tr>
              <td valign="top" style="border-bottom:1pt solid black;" align="left"><font face="Arial"><?php print $product->title; ?></font></td>
              <td valign="top" style="border-bottom:1pt solid black;" align="center"><font face="Arial"><?php print $product->qty; ?></font></td>
              <td valign="top" style="border-bottom:1pt solid black;" align="center"><font face="Arial"><?php print $product->price ?></font></td>
              <td valign="top" style="border-bottom:1pt solid black;" align="center"><font face="Arial"><?php print $product->rebate ?></font></td>
              <td valign="top" style="border-left:1pt solid #ccc; border-bottom: 1pt solid black;" align="center"><font face="Arial"><?php print $product->total_price; ?></font></td>
            </tr>
            <?php endforeach; ?>
  </table>     
          </td>
          
        
      </tr>
      <tr>
        <td colspan="3">
        
        <table width="100%">
        <td valign="top" class="customer_info_body" id="navcontainer3">
        
        <b><?php print t('Customer Info:'); ?></b><br /><br />
        
       
          <?php print $order_first_name ;?> <?php print $order_last_name ;?><br />
          <?php print $order_billing_phone; ?><br />
          <!--<?php //print $order_billing_address; ?><br />-->
          <?php print $order_email; ?><br />
          
		</td>
          
        <td valign="top" style="border-left:1pt solid #ccc;">
		
		<b><?php print t('Local Store:'); ?></b><br /><br />
         <?php print $cs_store_info; ?>
        
        </td>
        
        <td bgcolor="#E11D2F" style="color:#FFFFFF; font-size: 38px;" colspan="2" align="center"><font face="Arial"><?php print $order_total; ?><br /><span style="color:white; font-size: 12px;">Local taxes apply</span></font></td>
        </table>
        
        
        </td>
      </tr>
      <tr>
        <td colspan="3" bgcolor="#cd091f" style="color:#FFFFFF; font-size: 18px;" class="storeinfo"><font face="Arial"><center>Know someone looking for a career in sales? <a href="http://www.joincellularsales.com" style="color:#FFFFFF;">JoinCellularSales.com</a></center></font></td>

      </tr>

      <tr>
        <td colspan="3"><center><a href="https://facebook.com/CellularSales"><img alt="" src="<?php print $base_url; ?>/sites/all/themes/cellsales7/img/facebook_icon.png" style="width: 55px; height: 55px;"></a><a href="https://twitter.com/CellularSales" style="line-height: 1.538em;"><img alt="Twitter" src="<?php print $base_url; ?>/sites/all/themes/cellsales7/img/twitter_icon.png" style="width: 55px; height: 55px;"></a><a href="https://www.youtube.com/user/CellularSalesVideo" style="line-height: 1.538em;"><img alt="Youtube" src="<?php print $base_url; ?>/sites/all/themes/cellsales7/img/youtube_icon.png" style="width: 55px; height: 55px;"></a><a href="http://www.cellularsalesblog.com" style="line-height: 1.538em;"><img alt="CS Blog" src="<?php print $base_url; ?>/sites/all/themes/cellsales7/img/cs_blog_icon.png" style="width: 55px; height: 55px;"></a><a href="https://www.linkedin.com/company/cellular-sales" style="line-height: 1.538em; text-decoration: underline;"><img alt="LinkedIn" src="<?php print $base_url; ?>/sites/all/themes/cellsales7/img/linkedin_icon.png" style="width: 55px; height: 55px;"></a></center></td>

      </tr>

      <tr>
        <td colspan="3" style="color:#777"><font face="Arial"><center>&copy;2014 CELLULARSALES - 9040 EXECUTIVE PARK DRIVE - KNOXVILLE, TN 37923</center></font></td>
      </tr>
      <tr>
        <td colspan="3" style="color:#777"><font face="Arial"><center><a href="<?php print $base_url; ?>/unsubscribe-email" style="color: #777777;">Unsubscribe</a> | <a href="<?php print $base_url; ?>/legal/sms" style="color: #777777;">Privacy Policy</a></center></font></td>

      </tr>
      <tr>
        <td colspan="3"><center><img src="<?php print $base_url; ?>/sites/default/files/email_logo2.jpg" alt="Cellularsales" /></center></td>
      </tr>

    </table>
    </center>

    </td>
  </tr>
</table>

</body>
</html>