<?php

define ('PLUGIN_NAME', 'easydigitaldownloads');
define ('PLUGIN_VERSION', '3.0');
define ('BASE_URL', "https://core.jeeb.io/api/");

// Log the errors and callbacks for debugging
function jeeb_log($contents)
{
    error_log($contents);
}


function convert_base_to_bitcoin($base_currency, $amount)
{
    $ch = curl_init(BASE_URL . 'currency?value=' . $amount . '&base=' . $base_currency . '&target=btc');
    curl_setopt($ch, CURLOPT_CUSTOMREQUEST, "GET");
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_HTTPHEADER, array(
        'Content-Type: application/json',
        'User-Agent:' . PLUGIN_NAME . '/' . PLUGIN_VERSION)
    );
    $result = curl_exec($ch);
    $data = json_decode($result, true);
    error_log('Response =>'. var_export($data, TRUE));
    return (float) $data["result"];
}

function create_payment($signature, $options = array())
{
    $post = json_encode($options);
    $ch = curl_init(BASE_URL . 'payments/' . $signature . '/issue/');
    curl_setopt($ch, CURLOPT_CUSTOMREQUEST, "POST");
    curl_setopt($ch, CURLOPT_POSTFIELDS, $post);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_HTTPHEADER, array(
        'Content-Type:application/json',
        'User-Agent:' . PLUGIN_NAME . '/' . PLUGIN_VERSION,
    ));
    $result = curl_exec($ch);
    $data = json_decode($result, true);
    error_log('Response =>'. var_export($data, TRUE));
    return $data['result']['token'];
}

function confirm_payment($signature, $options = array())
{
    $post = json_encode($options);
    $ch = curl_init(BASE_URL . 'payments/' . $signature . '/confirm/');
    curl_setopt($ch, CURLOPT_CUSTOMREQUEST, "POST");
    curl_setopt($ch, CURLOPT_POSTFIELDS, $post);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_HTTPHEADER, array(
        'Content-Type:application/json',
        'User-Agent:' . PLUGIN_NAME . '/' . PLUGIN_VERSION,
    ));
    $result = curl_exec($ch);
    $data = json_decode($result, true);
    error_log('Response =>'. var_export($data, TRUE));
    return (bool) $data['result']['isConfirmed'];
}

function redirect_payment($token)
{
  // Using Auto-submit form to redirect user with the token
  return "<form id='form' method='post' action='".BASE_URL."payments/invoice'>".
          "<input type='hidden' autocomplete='off' name='token' value='".$token."'/>".
         "</form>".
         "<script type='text/javascript'>".
              "document.getElementById('form').submit();".
         "</script>";
}
