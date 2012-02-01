<?php

/**
 * Displays the list of current vhost objects.
 */
function vhost_view_objects() {
  $output = '';
  drupal_set_title(t('Manage Vhost Objects'));
  
  $object_list = array();
  
  $result = db_query("SELECT * FROM {vhost_objects} WHERE oid > 0 ORDER BY server_name");
  
  foreach ($result as $object) {
    $columns = array(
      $object->server_name,
      l(t('Edit'), 'admin/vhost/edit/' . $object->oid),
    );
    
    $object_list[] = $columns;
  }
  
  $header = array(
    array('data' => t('ServerName')),
    array('data' => t('Operations')),
  );
  
  $build['vhost_objects'] = array(
    '#theme' => 'vhost_view_objects',
    '#object_list' => (!empty($object_list)? theme('table', array('header' => $header, 'rows' => $object_list)):t('Sorry, no sites have been added yet.')),
  );
  
  return $build;
}


/**
 * Form builder; Configure Vhost settings.
 *
 * @ingroup forms
 * @see system_settings_form()
 */
function vhost_settings_form() {
  $form['http'] = array(
    '#type' => 'fieldset',
    '#title' => t('Server Settings'),
  );
  
  $form['http']['vhost_document_root_prefix'] = array(
    '#type' => 'textfield',
    '#title' => t('DocmuentRoot Base Path'),
    '#description' => t('The absolute filepath to the http docs.'),
    '#default_value' => variable_get('vhost_document_root_prefix', ''),
    '#required' => TRUE,
  );
  
  $form['http']['vhost_sites_conf_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Sites.d Path'),
    '#description' => t('The absolute path to the sites.d folder.'),
    '#default_value' => variable_get('vhost_sites_conf_path', ''),
    '#required' => TRUE,
  );
  
  $form['http']['vhost_log_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Custom Logs Path'),
    '#description' => t('The absolute path to the custom logs folders.'),
    '#default_value' => variable_get('vhost_log_path', ''),
    '#required' => TRUE,
  );
  
  $form['http']['vhost_restart_script_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Restart Script File'),
    '#description' => t('The absolute path to the custom script to gracefully restart apache.'),
    '#default_value' => variable_get('vhost_restart_script_path', ''),
    '#required' => TRUE,
  );
  
  return system_settings_form($form);
}

/**
 * Validator for vhost_settings_form().
 *
 * @see vhost_settings_form().
 */
function vhost_settings_form_validate($form, &$form_state) {
  if (substr($form_state['values']['vhost_document_root_prefix'], -1, 1) != '/') {
    $form_state['values']['vhost_document_root_prefix'] .=  '/';
  }
  
  if (substr($form_state['values']['vhost_sites_conf_path'], -1, 1) != '/') {
    $form_state['values']['vhost_sites_conf_path'] .= '/';
  }
  
  if (substr($form_state['values']['vhost_log_path'], -1, 1) != '/') {
    $form_state['values']['vhost_log_path'] .= '/';
  }
  
  if (substr($form_state['values']['vhost_restart_script_path'], -1, 1) == '/') {
    $form_state['values']['vhost_restart_script_path'] = substr($form_state['values']['vhost_restart_script_path'], 0, -1);
  }
}


/**
 * Form builder; Add/edit a virtual host.
 *
 * @ingroup forms
 */
function vhost_object_form($form, &$form_state, $object = NULL) {
  if (!$object) {
    $object = new stdClass;
    $object->document_root = '';
    $object->server_name = '';
    $object->server_alias = '';
    
    drupal_set_title(drupal_get_title() . t(' : Add VirtualHost'));
  } else {
    drupal_set_title(drupal_get_title() . t(' : Edit VirtualHost'));
    
    $form['oid'] = array(
      '#type' => 'hidden',
      '#value' => $object->oid,
    );
  }
  
  $form['vhost'] = array(
    '#type' => 'fieldset',
    '#title' => t('VirtualHost Settings'),
  );
  
  $form['vhost']['document_root'] = array(
    '#type' => 'textfield',
    '#title' => t('DocumentRoot'),
    '#description' => t('Prefixed with: ') . variable_get('vhost_document_root_prefix'),
    '#default_value' => $object->document_root,
    '#required' => TRUE,
  );
  
  $form['vhost']['server_name'] = array(
    '#type' => 'textfield',
    '#title' => t('ServerName'),
    '#description' => t('Also used for the machine name. Must be unique.'),
    '#default_value' => $object->server_name,
    '#required' => TRUE,
  );
  
  $form['vhost']['server_alias'] = array(
    '#type' => 'textfield',
    '#title' => t('ServerAlias'),
    '#description' => t('Separate multiple values with spaces.'),
    '#default_value' => $object->server_alias,
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  
  if (isset($object->oid)) {
    $form['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#submit' => array('vhost_object_form_delete'),
    );
  }
  
  return $form;
}

/**
 * Saves a vhost object to the database.
 */
function vhost_object_form_submit($form, &$form_state) {
  $object = array(
    'created' => REQUEST_TIME,
    'modified' => REQUEST_TIME,
    'document_root' => $form_state['values']['document_root'],
    'server_name' => $form_state['values']['server_name'],
    'server_alias' => $form_state['values']['server_alias'],
  );
  
  if (isset($form_state['values']['oid'])) {
    $object['oid'] = $form_state['values']['oid'];
    
    // don't alter the created date.
    unset($object['created']);
    
    db_update('vhost_objects')
      ->fields($object)
      ->condition('oid', $object['oid'])
      ->execute();
  } else {
    db_insert('vhost_objects')
      ->fields($object)
      ->execute();
  }
  
  vhost_create_vhost($object);
  
  drupal_set_message(t('Vhost entry saved.'));
  drupal_goto('admin/vhost');
}

function vhost_object_form_delete($form, $form_state) {
  db_delete('vhost_objects')
    ->condition('oid', $form_state['values']['oid'])
    ->execute();
  
  unlink(variable_get('vhost_sites_conf_path') . $form_state['values']['server_name'] . '.conf');
  
  drupal_set_message(t('VirtualHost deleted.'));
  
  drupal_goto('admin/vhost');
}