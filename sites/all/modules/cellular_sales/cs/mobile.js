(function ($) {

Drupal.behaviors.cs = {

  /**
   * Update the page based on GPS or other changes.
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

    $('#cs-nearest-store', context).each(function() {
      var refreshUrl = '/home-update/';
      var locationCookie = 'cs_location'; // Set by the cs module for the user's current location.
      var userLocationCookie = 'cs_user_location';
      var location = getCookie(locationCookie);
      var userLocation = getCookie(userLocationCookie);
      
      var refreshPage = function(location) {
        if (!location || !location.hasOwnProperty('addr')) {
          location = {};
        }
        $.ajax({
          url: refreshUrl,
          data: location,
          cache: false,
          success: function(data) {
            if (data && typeof(data) == 'object') {
              $('#cs-nearest-store').html(data.nearest_store);
              if ('store_list' in data) {
                $('#cs-store-list').html(data.store_list);
              }
              deleteCookie(userLocationCookie);
              setCookie(userLocationCookie, '1');
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
              refreshPage({
                addr: results[0].formatted_address,
                lat: coords.latitude,
                lng: coords.longitude
              });
            } else {
              refreshPage({
                addr: 'Your current location',
                lat: coords.latitude,
                lng: coords.longitude
              });
            }
          });
        }
      }
      
      if (!userLocation && navigator.geolocation) {
        navigator.geolocation.getCurrentPosition(getPosition, getPosition);
      }
      
    });
        
  } // attach: function
  
}

})(jQuery);