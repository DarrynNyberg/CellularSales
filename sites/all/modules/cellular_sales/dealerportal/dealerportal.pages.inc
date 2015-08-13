<?php
include_once("./".drupal_get_path('module','location')."/earth.inc");

//Copied from the "earth.inc" function inside the "location" module
function earth_distance_sql2($longitude, $latitude, $tbl_lat, $tbl_lng) {
  // Make a SQL expression that estimates the distance to the given location.
  $long = deg2rad($longitude);
  $lat = deg2rad($latitude);
  $radius = earth_radius($latitude);

  // If the table alias is specified, add on the separator.
  $tbl_alias = empty($tbl_alias) ? $tbl_alias : ($tbl_alias .'.');

  $coslong = cos($long);
  $coslat = cos($lat);
  $sinlong = sin($long);
  $sinlat = sin($lat);
  return "(IFNULL(ACOS($coslat*COS(RADIANS({$tbl_lat}))*($coslong*COS(RADIANS({$tbl_lng})) + $sinlong*SIN(RADIANS({$tbl_lng}))) + $sinlat*SIN(RADIANS({$tbl_lat}))), 0.00000)*$radius)";
}

function dealerportal_orderAdminChanged($sql = NULL, $args = NULL, $search = FALSE) {
  global $user;
  //Figure out current user/dealer's assigned store - (used to filter results)
  $sql = "SELECT ss.nid, l.latitude AS lat, l.longitude AS lng, ss.marketid AS market FROM {cs_store} ss ";
  $sql.= "LEFT JOIN {content_type_dealer_profile} cdp ON cdp.field_storelocation_nid = ss.nid ";
  $sql.= "LEFT JOIN {dealerportal_dealer} dd ON dd.nid = cdp.nid ";
  $sql.= "LEFT JOIN {users} u ON u.uid = dd.uid ";
  $sql.= "LEFT JOIN {location_instance} li ON li.nid = ss.nid ";
  $sql.= "LEFT JOIN {location} l ON li.lid = l.lid ";
  $sql.= "WHERE u.uid = %d";
  $DStoreNid = db_fetch_object(db_query($sql,$user->uid));
  
  //Setup the "pending orders" table's columns
  $header = array(
    array('data' => t('Actions')),
    array('data' => t('Assign To Me')),
    array('data' => t('Order ID')),
    array('data' => t('Customer')),
    $DStoreNid->nid==0?array('data' => t('Market')):array('data' => t('Distance~'), 'field' => 'dist', 'sort' => ''),
    array('data' => t('Date'), 'align' => 'center', 'field' => 'o.created', 'sort' => 'desc'),
    array('data' => t('ZIP')),
  );
  
  //Query table's data
  $sql = "SELECT o.order_id, o.uid, TRIM(CONCAT(o.billing_first_name,' ',o.billing_last_name)) AS ordername, ";
  $sql.= "  n.title AS dealername, u.name AS dealerUname, dq.market, ";
  if ($DStoreNid->nid!=0) $sql .= "ROUND(" . earth_distance_sql2(floatval($DStoreNid->lng),floatval($DStoreNid->lat),'dq.lat','dq.lng') . "*" . METER_2_MI . ",1) AS dist, ";
  $sql.= "  o.created, LEFT(o.billing_postal_code,5) AS zip, CONCAT(o.billing_city,', ',uz.zone_code) AS city ";
  $sql.= "FROM {uc_orders} o ";
  $sql.= "LEFT JOIN {uc_order_statuses} os ON o.order_status = os.order_status_id ";
  $sql.= "LEFT JOIN {uc_zones} uz ON uz.zone_id = o.billing_zone ";
  $sql.= "LEFT JOIN {dealerorders} `do` ON do.orderid = o.order_id ";
  $sql.= "LEFT JOIN {users} u ON u.uid = do.vendorid ";
  $sql.= "LEFT JOIN {dealerportal_dealer} dd ON dd.uid = do.vendorid ";
  $sql.= "LEFT JOIN {dealerportal_orderqueue} dq ON dq.orderid = o.order_id ";
  $sql.= "LEFT JOIN {node} n ON dd.nid = n.nid ";
  $sql.= "WHERE os.state = 'post_checkout' ";  //Don't filter on order_total > 0 !!!  There can be orders with 0 total.
  if ($DStoreNid->nid!=0) $sql.= "AND (dq.market = '{$DStoreNid->market}' OR do.vendorid={$user->uid}) ";
  //Set the sort order
  if (strcasecmp($_GET['order'],"date")==0) {
    $sql.= "ORDER BY o.created " . check_plain($_GET['sort']) . " ";
  } elseif (strcasecmp($_GET['order'],"distance~")==0) {
    $sql.= "ORDER BY dist " . check_plain($_GET['sort']) . " ";
  } else $sql.= "ORDER BY o.created DESC ";
  $result = pager_query($sql, 20, 0, NULL);
  
  $context = array(
    'revision' => 'themed-original',
    'type' => 'amount',
  );
  while ($order = db_fetch_object($result)) {
    $order_name = $order->ordername;
    if (empty($order_name)) {
      if ($order->uid !== 0) {
        $account = db_result(db_query("SELECT name FROM {users} WHERE uid = %d", $order->uid));
      }
      if (empty($account)) {
        $order_name = t('User: none');
      }
      else {
        $order_name = t('User: !name', array('!name' => $account));
      }
    }
    //Dealer's name
    if (empty($order->dealername)) {
      $venanme = $order->dealerUname;
    } else {
      $venanme = "<a href='/expert/{$order->dealerUname}' title='Profile page'>{$order->dealername}</a>";
    }
    if (!empty($order->dealerUname)){
      $ucactions = "";
      $checkboxorname = $venanme;
    } else {
      $ucactions = uc_order_actions($order, TRUE);
      $checkboxorname = "<input type=checkbox name=selorder[] value=".$order->order_id.">";
    }
    $rows[] = array(
      'data' => array(
        array('data' => $ucactions, 'nowrap' => 'nowrap'),
        array('data' => $checkboxorname),
        array('data' => "<a href='/admin/store/orders/{$order->order_id}' title='View order'>{$order->order_id}</a>"),
        array('data' => check_plain($order_name), 'nowrap' => 'nowrap'),
        $DStoreNid->nid==0?array('data' => $order->market):array('data' => $order->dist),
        array('data' => format_date($order->created, 'custom', variable_get('uc_date_format_default', 'm-d-Y')), 'align' => 'center'),
        array('data' => "<span title='{$order->city}'>{$order->zip}</span>"),
      ),
      'id' => 'order-'. $order->order_id,
    );
  }
  $output .= theme('table', $header, $rows, array('class' => 'uc-orders-table'));
  $output .= theme('pager', NULL, 2, 0);
  return $output;
}

function dealerportal_UserMarket() {
  global $user;
  //Lookup dealer's current market
  $sql = "SELECT ss.marketid AS market FROM {cs_store} ss ";
  $sql.= "INNER JOIN {content_type_dealer_profile} cdp ON cdp.field_storelocation_nid = ss.nid ";
  $sql.= "INNER JOIN {dealerportal_dealer} dd ON dd.nid = cdp.nid ";
  $sql.= "INNER JOIN {users} u ON u.uid = dd.uid ";
  $sql.= "WHERE u.uid=%d";
  return db_result(db_query($sql,$user->uid));
}

function dealerportal_PendingOrders() {
  global $user;
  $userMarket = dealerportal_UserMarket();
  //Display the title
  print "<div class='tabs-heading'>All Pending Orders";
  if (!empty($userMarket)) print " in Your Market:&nbsp;&nbsp;{$userMarket}";
  print "</div>";
  print "<div class='pending-orders-wrapper'>";
  //Check for authorization
  if (! in_array("dealer", $user->roles) && ! in_array("superdealer", $user->roles)) {
    print "You are not authorised to view this page";
    print "</div>";
    return;  //Exit
  }
  //Dealer order assignment, if applicable
  print "<table border='1' cellspacing='0' cellpadding='0' width='100%' BORDERCOLOR='#cccccc'><tr><td>";
  if($_GET["go"] == "Assign"){
    if(! db_table_exists("dealerorders")){
      db_query("CREATE TABLE dealerorders ('id' SERIAL NOT NULL ,'orderid' INT NOT NULL ,'vendorid' INT NOT NULL)");
    }
    for($i=0;$i<count($_GET["selorder"]);$i++){
      $sql = "INSERT INTO dealerorders SELECT NULL,".$_GET["selorder"][$i].",{$user->uid} FROM DUAL WHERE NOT EXISTS (SELECT * FROM dealerorders WHERE vendorid={$user->uid} AND orderid=".$_GET["selorder"][$i].")";
      db_query($sql);
    }
  }
  //Build table
  print "<form name='frm'>";
  print dealerportal_orderAdminChanged();
  print "<table border='0' width='100%'><tr><td>&nbsp;</td><td align=left><input type='submit' name='go' value='Assign' style='margin-left:80px'></td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td></tr></table>";
  print "</form>";
  print "</td></tr></table>";
  print "<script language='javascript'> function changestatus(myString) { document.frm.submit(); } </script>";
  print "</div>";
}