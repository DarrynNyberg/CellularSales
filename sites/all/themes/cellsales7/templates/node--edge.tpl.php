<?php
	// set vars
	$family_count = $node->field_family_of_x['und'][0]['value'];
	$price_everything = $node->field_family_price_per_phone['und'][0]['value'];
	$price_everything_edge = $node->field_family_price_perphone_edg['und'][0]['value'];
	$monthly_gb_price = $node->field_monthly_gb['und'][0]['value'];
	$individual = 1;
	$monthly_gb_price_individual = $node->field_monthly_2gb['und'][0]['value'];
	
	// set calc vars
	$price_everything_total = $family_count * $price_everything;
	$total_non_edge = $price_everything_total + $monthly_gb_price;
	
	$price_everything_edge_total = $family_count * $price_everything_edge;
	$total_with_edge = $price_everything_edge_total + $monthly_gb_price;
	
	$you_save = $price_everything_total - $price_everything_edge_total;
	

?>
<?php print render($content['body']); ?>
<h2 class="more-header">Verizon Edge helps you save</h2>
<?php print render($content['field_edge_details']); ?>


<div class="row pricing-tables">
	<div class="col-sm-6">
	
		<div class="col6-box" style="padding-right:10px; float:left;">
		    <h3 class="family-header">As a Family of <?php print $family_count; ?>:</h3>
		    <table>
		        <thead>
		            <tr>
		                <th class="col2-box"></th>
		                <th class="col1-box la-pricing" id="line-access">
		                    <a href="javascript:void(0)" class="tooltip-bottom tooltipsy questionLink upprCase fs11" data-tooltip="Monthly Line Access is the total cost of connecting all your devices to your plan. You may add up to 10 devices to your plan.">Line Access<span aria-hidden="true" class="icon-tooltip"></span></a>
		                    <div class="tip" style="display: none;">
		                        <p>Monthly Line Access is the total cost of connecting all your devices to your plan. You may add up to 10 devices to your plan.</p>
		                    </div>
		                </th>
		                <th class="col1-box da-pricing" id="data-access">
		                    <a href="javascript:void(0)" class="tooltip-bottom tooltipsy questionLink upprCase fs11" data-tooltip="Monthly Data Access is the cost for your data allowance. It's shared between all the devices on your plan.">Data Access<span aria-hidden="true" class="icon-tooltip"></span></a>
		                    <div class="tip" style="display: none;">
		                        <p>Monthly Data Access is the cost for your data allowance. It's shared between all the devices on your plan.</p>
		                    </div>
		                </th>
		                <th class="col2-box ta-pricing" id="total-access"><strong>Total Access</strong></th>
		            </tr>
		        </thead>
		        <tbody>
		            <tr class="white-bg">
		                <td class="col2-box row-header" id="tbl-more-everything">
		                    <p><strong><?php print $family_count; ?> Smartphones on MORE Everything</strong></p>
		                </td>
		                <td class="col1-box la-pricing" headers="line-access tbl-more-everything">
		                    <p class="price"><span class="price-prefix">
		                    	<?php print $family_count; ?> x $<?php print $price_everything; ?></span><br />
		                    	$<?php print $price_everything_total; ?><br />
		                    	<span class="price-details">monthly <span class="nowrap">line access</span></span>
		                    </p>
		                </td>
		                <td class="col1-box da-pricing" headers="data-access tbl-more-everything">
		                    <p class="price">
		                    	<span class="price-prefix">10 <abbr title="Gigabytes">GB</abbr></span><br />
		                    	$<?php print $monthly_gb_price; ?><br />
		                    	<span class="price-details">monthly account access</span>
		                    </p>
		                </td>
		                <td class="col2-box ta-pricing" headers="total-access tbl-more-everything">
		                    <p class="price no-prefix">
		                    	$<?php print $total_non_edge; ?><br />
		                    	<span class="price-details">monthly access*</span>
		                    </p>
		                </td>
		            </tr>
		            <tr class="red-bg">
		                <td class="col2-box red-bg white-text row-header" id="tbl-more-everything-edge">
		                    <p><strong><?php print $family_count; ?> Smartphones on MORE Everything with Edge</strong></p>
		                </td>
		                <td class="col1-box red-bg white-text la-pricing" headers="line-access tbl-more-everything-edge">
		                    <p class="price">
		                    	<span class="price-prefix"><?php print $family_count; ?> x $<?php print $price_everything_edge; ?></span><br />
		                    	$<?php print $price_everything_edge_total; ?><br />
		                    	<span class="price-details">monthly <span class="nowrap">line access</span></span>
		                    </p>
		                </td>
		                <td class="col1-box red-bg white-text da-pricing" headers="data-access tbl-more-everything-edge">
		                    <p class="price">
		                    	<span class="price-prefix">10 <abbr title="Gigabytes">GB</abbr></span><br />
		                    	$<?php print $monthly_gb_price; ?><br />
		                    	<span class="price-details">monthly account access</span>
		                    </p>
		                </td>
		                <td class="col2-box red-bg white-text ta-pricing" headers="total-access tbl-more-everything-edge">
		                    <p class="price">
		                    	<span class="price-prefix nowrap">Save $<?php print $you_save; ?></span><br />
		                    	$<?php print $total_with_edge; ?><br />
		                    	<span class="price-details">monthly access**</span>
		                    </p>
		                </td>
		            </tr>
		        </tbody>
		    </table>
		</div>
		
		
	</div>
	<div class="col-sm-6">
		
		
		<div class="col6-box" style="padding-right:10px; float:left;">
		    <h3 class="individual-header">As an Individual:</h3>
		    <table>
		        <thead>
		            <tr>
		                <th class="col2-box"></th>
		                <th class="col1-box la-pricing" id="line-access">
		                    <a href="javascript:void(0)" class="tooltip-bottom tooltipsy questionLink upprCase fs11" data-tooltip="Monthly Line Access is the total cost of connecting all your devices to your plan. You may add up to 10 devices to your plan.">Line Access<span aria-hidden="true" class="icon-tooltip"></span></a>
		                    <div class="tip" style="display: none;">
		                        <p>Monthly Line Access is the total cost of connecting all your devices to your plan. You may add up to 10 devices to your plan.</p>
		                    </div>
		                </th>
		                <th class="col1-box da-pricing" id="data-access">
		                    <a href="javascript:void(0)" class="tooltip-bottom tooltipsy questionLink upprCase fs11" data-tooltip="Monthly Data Access is the cost for your data allowance. It's shared between all the devices on your plan.">Data Access<span aria-hidden="true" class="icon-tooltip"></span></a>
		                    <div class="tip" style="display: none;">
		                        <p>Monthly Data Access is the cost for your data allowance. It's shared between all the devices on your plan.</p>
		                    </div>
		                </th>
		                <th class="col2-box ta-pricing" id="total-access"><strong>Total Access</strong></th>
		            </tr>
		        </thead>
		        <tbody>
		            <tr class="white-bg">
		                <td class="col2-box row-header" id="tbl-more-everything">
		                    <p><strong><?php print $individual; ?> Smartphone on MORE Everything</strong></p>
		                </td>
		                <td class="col1-box la-pricing" headers="line-access tbl-more-everything">
		                    <p class="price"><span class="price-prefix">
		                    	<?php print $individual; ?> x $<?php print $price_everything; ?></span><br />
		                    	$<?php print $price_everything_total; ?><br />
		                    	<span class="price-details">monthly <span class="nowrap">line access</span></span>
		                    </p>
		                </td>
		                <td class="col1-box da-pricing" headers="data-access tbl-more-everything">
		                    <p class="price">
		                    	<span class="price-prefix">10 <abbr title="Gigabytes">GB</abbr></span><br />
		                    	$<?php print $monthly_gb_price; ?><br />
		                    	<span class="price-details">monthly account access</span>
		                    </p>
		                </td>
		                <td class="col2-box ta-pricing" headers="total-access tbl-more-everything">
		                    <p class="price no-prefix">
		                    	$<?php print $total_non_edge; ?><br />
		                    	<span class="price-details">monthly access*</span>
		                    </p>
		                </td>
		            </tr>
		            <tr class="red-bg">
		                <td class="col2-box red-bg white-text row-header" id="tbl-more-everything-edge">
		                    <p><strong><?php print $individual; ?> Smartphone on MORE Everything with Edge</strong></p>
		                </td>
		                <td class="col1-box red-bg white-text la-pricing" headers="line-access tbl-more-everything-edge">
		                    <p class="price">
		                    	<span class="price-prefix"><?php print $individual; ?> x $<?php print $price_everything_edge; ?></span><br />
		                    	$<?php print $individual * $price_everything_edge; ?><br />
		                    	<span class="price-details">monthly <span class="nowrap">line access</span></span>
		                    </p>
		                </td>
		                <td class="col1-box red-bg white-text da-pricing" headers="data-access tbl-more-everything-edge">
		                    <p class="price">
		                    	<span class="price-prefix">2 <abbr title="Gigabytes">GB</abbr></span><br />
		                    	$<?php print $monthly_gb_price_individual; ?><br />
		                    	<span class="price-details">monthly account access</span>
		                    </p>
		                </td>
		                <td class="col2-box red-bg white-text ta-pricing" headers="total-access tbl-more-everything-edge">
		                    <p class="price">
		                    	<span class="price-prefix nowrap">Save $<?php print $you_save; ?></span><br />
		                    	$<?php print $monthly_gb_price_individual + ($individual * $price_everything_edge); ?><br />
		                    	<span class="price-details">monthly access**</span>
		                    </p>
		                </td>
		            </tr>
		        </tbody>
		    </table>
		</div>
		
		
	</div>
</div>
