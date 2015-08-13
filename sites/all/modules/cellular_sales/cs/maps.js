(function ($) {

Drupal.behaviors.cs_maps = {
  
  attach: function (context, settings) {
    // Parameter data converted from PHP.
    var mapData = Drupal.settings.cs_map.markers;
    var maxDistance = Drupal.settings.cs_map.max_distance;
    var defaultZoom = Drupal.settings.cs_map.zoom;
  
    var markerCount = mapData.length;
    var mapCenter;
    var points = [];
    var bounds = new google.maps.LatLngBounds();
    var boundsCount = 0;
    var fitBounds = false;
    var locationQuery = markerCount < 1 || !mapData[0].nid; // Node id if a market or store page query.
  
    for (var i = 0; i < markerCount; i++) {
      points[i] = new google.maps.LatLng(mapData[i].lat, mapData[i].lon);
      mapData[i].point = points[i];
      if (maxDistance == 0 || mapData[i].distance <= maxDistance) {
        bounds.extend(points[i]);
        boundsCount++;
      }
    }
  
    // None of the other markers were within the distance. Zoom out to include nearest 4 to center point.
    if (locationQuery && maxDistance > 0 && boundsCount < 2 && markerCount >= 2) {
      fitBounds = true;
      for (var i = 1; i < markerCount && i <= 4; i++) {
        bounds.extend(points[i]);
        boundsCount++;
      }
    }

    if (boundsCount > 1 && maxDistance == 0) { // Market queries, center on all.
      mapCenter = bounds.getCenter();
      fitBounds = true;
    } else {
      mapCenter = points[0];
    }

    var mapOptions = {
      zoom: defaultZoom,
      center: mapCenter,
      mapTypeId: google.maps.MapTypeId.ROADMAP,
      panControl: true,
      zoomControl: true,
      mapTypeControl: true,
      scaleControl: false,
      streetViewControl: false,
      overviewMapcontrol: false
    };
    var map = new google.maps.Map(document.getElementById('googmap'), mapOptions);
    if (fitBounds) {
      /* 
      * We can't get the zoom after map.fitBounds - have to get it asynchronously.
      * Ended up not needing this, but leaving it here just in case. 
      * 
      google.maps.event.addListenerOnce(map, 'zoom_changed', function() {
        google.maps.event.addListenerOnce(map, 'bounds_changed', function(event) {
          if (this.getZoom() < defaultZoom) {
            this.setZoom(defaultZoom);
            this.setCenter(mapCenter);
          }
        });
      });
      */
      map.fitBounds(bounds);
    }

    var infoWindow = new google.maps.InfoWindow();
    var infoClickFunction = function () {
      var mapData = this.storelocMapData;
      infoWindow.setOptions({content: mapData.html, position: mapData.point});
      infoWindow.open(map);
    }

    var markers = [];
    var sideMarker; // Equivalent marker in the page left margin.
    for(var i = 0; i < points.length; i++) { 
      markers[i] = new google.maps.Marker({
        position: points[i],
        map: map,
        icon: mapData[i].icon
      });
      markers[i].storelocMapData = mapData[i];
      google.maps.event.addDomListener(markers[i], 'click', infoClickFunction);
      sideMarker = document.getElementById('storeloc_mark' + i);
      if (sideMarker) {
        sideMarker.storelocMapData = mapData[i];
        google.maps.event.addDomListener(sideMarker, 'click', infoClickFunction);
      }
    }

    infoWindow.setOptions({content: mapData[0].html, position: points[0]});
    infoWindow.open(map);
    
 } // End attach function

}

})(jQuery);