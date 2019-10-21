<?php

require_once("jeeb_lib.php");

/**
 * Process Invoice Status Notifiations from Jeeb.
 */
function uc_jeeb_notification()
{
  $postdata  = file_get_contents("php://input");
  $json      = json_decode($postdata, true);
  $signature = $json['signature'];

  jeeb_log("Entered Jeeb-Notification");
  if ( $json['stateId']== 2 ) {
    jeeb_log('Order Id received = '.$json['orderNo'].' stateId = '.$json['stateId']);$order = uc_order_load($json["orderNo"]);
    $order = uc_order_load($json["orderNo"]);
    uc_order_comment_save($order->order_id, 0, t('Invoice has been created.'), 'admin');
    $order->order_status = 'pending';
    uc_order_save($order);
  }
  else if ( $json['stateId']== 3 ) {
    jeeb_log('Order Id received = '.$json['orderNo'].' stateId = '.$json['stateId']);
    uc_cart_empty(uc_cart_get_id());
    $order = uc_order_load($json["orderNo"]);
    uc_order_comment_save($order->order_id, 0, t('Payment was received by Jeeb.'), 'admin');
    uc_order_comment_save($order->order_id, 0, t('Awaiting bitcoin confirmation'), 'admin');
    $order->order_status = 'payment_received';
    uc_order_save($order);
  }
  else if ( $json['stateId']== 4 ) {
    jeeb_log('Order Id received = '.$json['orderNo'].' stateId = '.$json['stateId']);
    $order = uc_order_load($json["orderNo"]);
    // $token =  db_result(db_query("SELECT token FROM {uc_payment_jeeb} WHERE order_id = %d", $order->order_id));
    $token = $json["token"];
    $data = array(
      "token" => $token,
    );

    $is_confirmed = confirm_payment($signature, $data);

    if($is_confirmed){
      jeeb_log('Payment confirmed by jeeb');
      $comment = t('Secure Payment ID: @id confirmed by Jeeb.', array('@id' => $json["referenceNo"]));
      uc_payment_enter($order->order_id, 'jeeb', uc_order_get_total($order), $order->uid, NULL, $comment, time());
      uc_order_comment_save($order->order_id, 0, t('Payment of @amount was confirmed by Jeeb. Secure Payment ID: @refId confirmed by Jeeb.', array('@amount' => $json["value"], '@refId' => $json["referenceNo"])), 'admin');

      $order->order_status = 'completed';
      uc_order_save($order);

    }
    else {
      jeeb_log('Payment rejected by jeeb');
    }
  }
  else if ( $json['stateId']== 5 ) {
    jeeb_log('Order Id received = '.$json['orderNo'].' stateId = '.$json['stateId']);
    $order = uc_order_load($json["orderNo"]);
    uc_order_comment_save($order->order_id, 0, t('Invoice was expired'), 'admin');
    $order->order_status = 'canceled';
    uc_order_save($order);
  }
  else if ( $json['stateId']== 6 ) {
    jeeb_log('Order Id received = '.$json['orderNo'].' stateId = '.$json['stateId']);
    $order = uc_order_load($json["orderNo"]);
    uc_order_comment_save($order->order_id, 0, t('Over payment occurred'), 'admin');
    $order->order_status = 'canceled';
    uc_order_save($order);
  }
  else if ( $json['stateId']== 7 ) {
    jeeb_log('Order Id received = '.$json['orderNo'].' stateId = '.$json['stateId']);
    $order = uc_order_load($json["orderNo"]);
    uc_order_comment_save($order->order_id, 0, t('Under payment occurred'), 'admin');
    $order->order_status = 'canceled';
    uc_order_save($order);
  }
  else{
    jeeb_log('Cannot read state id sent by Jeeb');
    $order = uc_order_load($json["orderNo"]);
    uc_order_comment_save($order->order_id, 0, t('Cannot read StateId. Contact Jeeb immediately.'), 'admin');
    $order->order_status = 'canceled';
    uc_order_save($order);
  }

} // end function - uc_jeeb_notification

// Handle the callback from the Jeeb server.
function uc_jeeb_callback(){
  jeeb_log("Entered callback with Order No. : ".$_POST["orderNo"]);
  jeeb_log("Data :".var_export($_POST, TRUE));
  $order = uc_order_load($_POST["orderNo"]);
  if($_POST["stateId"]==3){
    uc_cart_empty(uc_cart_get_id());
    drupal_set_message(t('Thank you for your order! Your order is now awaiting confirmation from Bitcoin Network.'));
    drupal_goto('cart');
  }
  else if($_POST["stateId"]==5){
    drupal_set_message(t('Your Jeeb Invoice was expired. Please feel free to continue shopping or contact us for assistance.'));
    drupal_goto('cart');
  }
  else if($_POST["stateId"]==6){
    drupal_set_message(t('Your Jeeb Invoice was over paid and was rejected by Jeeb. Please feel free to continue shopping or contact us for assistance.'));
    drupal_goto('cart');
  }
  else if($_POST["stateId"]==7){
    drupal_set_message(t('Your Jeeb Invoice was under paid and was rejected by Jeeb. Please feel free to continue shopping or contact us for assistance.'));
    drupal_goto('cart');
  }
} // end function uc_jeeb_callback
