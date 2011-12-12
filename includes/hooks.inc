<?php

/**
* Implementation of hook_menu()
*/
function aweber_menu() {
  $items = array();

  $items['aweberreturnpage'] = array(
    'title' => 'AWeber return',
    'page callback' => 'aweber_return_page',
    'access callback' => TRUE,
    'type' => MENU_CALLBACK
  );

  $items['admin/user/aweber'] = array(
    'title' => 'AWeber',
    'page callback' => 'aweber_overview',
    'description' => 'Overview of AWeber leads and integration settings',
    'access arguments' => array('access administration pages'),
    'type' => MENU_NORMAL_ITEM
  );
  $items['admin/user/aweber/list'] = array(
    'title' => 'Leads',
    'type' => MENU_DEFAULT_LOCAL_TASK,
    'weight' => -10
  );
  $items['admin/user/aweber/settings'] = array(
    'title' => 'Settings',
    'page callback' => 'drupal_get_form',
    'page arguments' => array('aweber_configure'),
    'access arguments' => array('access administration pages'),
    'type' => MENU_LOCAL_TASK
  );
  $items['admin/user/aweber/edit/%aweber_lead'] = array(
    'title' => 'Edit lead status',
    'page callback' => 'drupal_get_form',
    'page arguments' => array('aweber_lead_edit_form'),
    'access arguments' => array('access administration pages'),
    'type' => MENU_CALLBACK
  );
  $items['admin/user/aweber/delete/%aweber_lead'] = array(
    'title' => 'Delete lead',
    'page callback' => 'drupal_get_form',
    'page arguments' => array('aweber_lead_delete_form'),
    'access arguments' => array('access administration pages'),
    'type' => MENU_CALLBACK
  );

  return $items;
}

/**
* Implementation of hook_user()
*/
function aweber_user($op, &$edit, &$account, $category = NULL) {
  switch ($op) {
    case 'register':
      if (variable_get('aweber_user_form', 0) && variable_get('aweber_id', '') != '') {
        $form['aweber'] = array(
          '#type' => 'fieldset',
          '#title' => t('Newsletter'),
          '#collapsible' => TRUE,
          '#collapsed' => FALSE
        );
        $form['aweber']['aweber_signup'] = array(
          '#type' => 'checkbox',
          '#title' => filter_xss_admin(variable_get('aweber_option_text', t('Sign up for our newsletter.'))),
          '#default_value' => variable_get('aweber_registration_default', 0),
        );
        return $form;
      }
      break;
    case 'form':
      if (variable_get('aweber_user_form', 0) && variable_get('aweber_id', '') != '' && $category == 'account') {
        $form['aweber'] = array(
          '#type' => 'fieldset',
          '#title' => t('Newsletter'),
          '#collapsible' => TRUE,
          '#collapsed' => FALSE
        );
        $form['aweber']['aweber_signup'] = array(
          '#type' => 'checkbox',
          '#title' => filter_xss_admin(variable_get('aweber_option_text', t('Sign up for our newsletter.'))),
          '#default_value' => !empty($account->aweber_signup) ? $account->aweber_signup : 0,
        );
        return $form;
      }
      break;
    case 'update':
      if (isset($edit['mail']) && $edit['mail'] != $account->mail && $account->aweber_signup == 1) {
        aweber_update_mail($edit['mail'], $account->uid);
      }
      // purposefully fall through
    case 'insert':
      if (isset($edit['aweber_signup']) && $edit['aweber_signup'] == 1 && $account->aweber_signup == 0) {
        aweber_form_post($account->mail, $account->uid);
      }
      else if (isset($edit['aweber_signup']) && $edit['aweber_signup'] == 0 && $account->aweber_signup == 1) {
        aweber_update_lead_status(AWEBER_UNSUBSCRIBED, $account->uid);
      }
      break;
    case 'delete':
      if ($account->aweber_signup == 1) {
        aweber_update_lead_status(AWEBER_DELETED, $account->uid);
      }
      break;
  }
}


/**
* Implementation of hook_block()
*/
function aweber_block($op = 'list', $delta = 0, $edit = array()) {
  global $user;

  switch ($op) {
    case 'list':
      $blocks['aweber_form'] = array('info' => t('AWeber Sign-up Form'));
      return $blocks;

    case 'configure':
      $form['aweber_block_text'] = array(
        '#type' => 'textarea',
        '#title' => t('Description'),
        '#description' => t('Text to display above the form.'),
        '#default_value' => variable_get('aweber_block_text', '')
      );
      return $form;

    case 'save':
      variable_set('aweber_block_text', $edit['aweber_block_text']);
      break;

    case 'view':
      if ($delta == 'aweber_form' && ($user->uid == 0 || $user->aweber_signup == 0)) {
        $block['subject'] = t('Newsletter');
        if (variable_get('aweber_block_text', '') != '') {
          $block['content'] = theme('aweber_block_text', variable_get('aweber_block_text', ''));
          $block['content'] .= drupal_get_form('aweber_web_form');
        }
        else {
          $block['content'] = drupal_get_form('aweber_web_form');
        }
        return $block;
      }
      break;
  }
}


/**
 * Implementation of hook_theme().
 */
function aweber_theme() {
  return array(
    'aweber_block_text' => array(
      'arguments' => array('text' => NULL),
    ),
  );
}