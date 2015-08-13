/**
 *  Client side functions for cart pages
 *  @author tahira
 */
(function ($) {
    Drupal.behaviors.cart = {
        attach: function (context, settings) {

            if ($('#uc-cart-view-form').length !== 0) {
                $('#uc-cart-view-form table tr > :nth-child(4)').append(function () {
                    return $(this).prev().remove().contents();
                });
                
                $('div.entity-uc-cart-item').remove();
                
                $("select").uniform();
            }
            
            if ($('.product-info input[name="price-plan"]').length !== 0) {
                $('.product-info input[name="price-plan"]').on('change', function () {
                    var selected_plan = $('.product-info input[name="price-plan"]:checked').val();
                    $('.add-to-cart input[name="price_plan"]').val(selected_plan);
                });
            }
        }
    };
})(jQuery);
