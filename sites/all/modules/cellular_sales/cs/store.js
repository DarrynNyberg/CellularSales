(function ($) {

/**
 * Javscript features for online store.
 */
Drupal.behaviors.ms = {
  attach: function (context) {
    var compute_plan_totals = function() {
      var line_price = [40, 30, 10, 5, 20]; //! Temp - need to pass data from drupal -- see maps.js.
      var total = $('#edit-monthly-total');
  		var total_edge = $('#edit-monthly-total-edge');
  		var devices = $('select[id^=edit-device-]');
  		var data_package = $('#edit-data-package')[0];
  		var quantity, price;
  		
  		var selected = data_package.selectedIndex;
  		var data_price = +data_package.options[selected].value;
  		
  		var sum = data_price;
  		var sum_edge = data_price;
  		
  		for (var i = 0; i < devices.length; i++) {
  		  selected = devices[i].selectedIndex;
  		  quantity = +devices[i].options[selected].value; // Unary plus converts to integer.
  		  price = line_price[i] * quantity;
  		  sum += price;
  		  sum_edge += price;
  		  if (i == 0 && quantity > 0) { // Edge discount applies only to smart phones.
  		    if (data_price >= 100) {
      		  sum_edge -= 25 * quantity;
      		} else {
      		  sum_edge -= 10 * quantity;
      		}
  		  }
  		}
  		
  		total.val('$' + sum);
  		total_edge.val('$' + sum_edge);
    }
    
    $('select[id^=edit-device-]', context)
  	.change(function() {
  		compute_plan_totals();
  	});
    
  	$('#edit-data-package', context)
  	.change(function() {
  		compute_plan_totals();
  	});
  }
}

})(jQuery);