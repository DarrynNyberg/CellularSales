(function ($) {

Drupal.behaviors.cs = {

  /**
   * Update store location blocks.
   * 
   * Even if block caching is off, Drupal won't update the block if the entire page is cached.
   * This refreshes the cached block for anonymous users after the page is loaded.
   */
  attach: function (context, settings) {
    var setCookie = function(name, value, days) {
      if (days) {
        var date = new Date();
        date.setTime(date.getTime()+(days*24*60*60*1000));
        var expires = '; expires='+date.toGMTString();
      } else {
        var expires = "";
      }
       document.cookie = name + "=" + escape(value) + expires + '; path=/ ;';
    }
    
    var getCookie = function(name) {
      var cValue = '';
      var jar = document.cookie.split(';');
      var cookie, cName;
      for (var i = 0; i < jar.length; i++){
        cookie = jar[i].split('=');
        cName = cookie[0].replace(/^\s+|\s+$/g, '');
        if (cName === name) {
          cValue = unescape(cookie[1]);
          break;
        }
      }
      return cValue;
    }

    var deleteCookie = function(name) {
      setCookie(name, '', -1);
    }

    $('#block-cs-store-locator,#block-cs-nearest-store', context).each(function() {
      var block = $(this).attr('id');
      block = block.indexOf('nearest-store') >= 0 ? 'nearest_store' : 'store_locator';
      var refreshUrl = '/cs-refresh-block/' + block;
      var blockCookie = 'cs_block_' + block;
      var locationCookie = 'cs_location'; // Set by the cs module for the user's current location.
      var lastLocationCookie = 'cs_last_location';
      var blockContent = getCookie(blockCookie);
      var location = getCookie(locationCookie);
      var lastLocation = getCookie(lastLocationCookie);
            
      if (!location || location !== lastLocation) {
        blockContent = '';
        deleteCookie('cs_block_store_locator');
        deleteCookie('cs_block_nearest_store');
      }
      
      var refreshBlock = function(location) {
        if (!location || !location.hasOwnProperty('addr')) {
          location = {};
        }
        $.ajax({
          url: refreshUrl,
          data: location,
          cache: false,
          success: function(data) {
            if (data) {
              $('#cs-nearest-store').html(data);
              deleteCookie(blockCookie);
              deleteCookie(lastLocationCookie);
              setCookie(blockCookie, data);
              setCookie(lastLocationCookie, getCookie(locationCookie));
            }
          }
        });
      }
      
      var getPosition = function(position) {
        var coords = position.coords || position.coordinate || position;
        if ('latitude' in coords) {
          var latLng = new google.maps.LatLng(coords.latitude, coords.longitude);
          var geo = new google.maps.Geocoder();
          geo.geocode({latLng: latLng}, function(results, status) {
            if (status == google.maps.GeocoderStatus.OK && 'formatted_address' in results[0]) {
              refreshBlock({
                addr: results[0].formatted_address,
                lat: coords.latitude,
                lng: coords.longitude
              });
            } else {
              refreshBlock();
            }
          });
        } else {
          refreshBlock();
        } 
      }
      
      if (blockContent) {
        $('#cs-nearest-store').html(blockContent);
      } else if (!lastLocation && navigator.geolocation) {
        // Tested adding try with refreshBlock catch, but it was never called even when network event was blocked.
        navigator.geolocation.getCurrentPosition(getPosition, refreshBlock);
      } else {
        refreshBlock();
      }
    });
        
  } // attach: function
  
}

})(jQuery);