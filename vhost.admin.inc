<?php

/**
 * Displays the list of current vhost objects.
 */
function vhost_view_objects() {
  $output = '';
  drupal_set_title(t('Manage vhost objects'));

  drupal_add_js('
    (function ($){
      $(document).ready(function(){
        $("#vhost-index a").click(function(e) {
          $("html, body").animate({
            scrollTop: $($(this).attr("href").slice(1)).offset().top - 60
          }, 1000);
          e.preventDefault();
        });
      })
    })(jQuery);
  ', 'inline');

  $object_list = array();
  $char_match = NULL;
  $index_used = array();
  $count = 0;

  $result = db_query("SELECT * FROM {vhost_objects} WHERE oid > 0 ORDER BY server_name");

  foreach ($result as $object) {
    $count++;
    $first_char = substr($object->server_name, 0, 1);
    if (!ctype_alpha($first_char)) {
      if (is_null($char_match)) {
        $object_list[] = array(
          'data' => array(
            array(
              'data' => '*',
              'colspan' => 2,
            ),
          ),
          'id' => 'group-*',
          'class' => array('group'),
        );

        // Record the use of *.
        $index_used[] = '*';
      }
    }
    else if ($first_char != $char_match) {
      $object_list[] = array(
        'data' => array(
          array(
            'data' => '<h2>' . strtoupper($first_char) . '</h2>',
            'colspan' => 2
          ),
        ),
        'id' => 'group-' . $first_char,
        'class' => array('group'),
      );

      // Record the use of $first_char.
      $index_used[] = strtoupper($first_char);
    }

    $char_match = $first_char;

    $columns = array(
      l($object->server_name, 'http://' . $object->server_name, array('attributes' => array('target' => '_blank'))),
      l(t('Edit'), 'admin/vhost/edit/' . $object->oid),
    );
    $object_list[] = $columns;
  }

  $header = array(
    array('data' => t('ServerName')),
    array('data' => t('Operations')),
  );

  // Start the index with *.
  $index = array('*');

  // Loop in alpha characters a-z.
  for ($i = 97; $i <= 122; $i++) {
    $index[] = strtoupper(chr($i));
  }

  // Link used characters.
  foreach ($index as $key => $value) {
    if (in_array($value, $index_used)) {
      $index[$key] = l($value, '', array('fragment' => 'group-' . strtolower($value)));
    }
  }

  $index = implode(' | ', $index);

  $build['vhost_index'] = array(
    '#markup' => '<p id="vhost-index">' . t('Index: ') . $index . '</p>',
  );

  $build['vhost_count'] = array(
    '#markup' => '<p><strong>' . format_plural($count, '@count Domain', '@count Domains') . '</strong></p>',
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
    '#autocomplete_path' => 'admin/vhost/autocomplete',
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
    '#type' => 'textarea',
    '#title' => t('ServerAlias'),
    '#description' => t('One entry per line.'),
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

  $sudo_password = NULL;

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

  if (isset($form_state['values']['sudo_password'])) {
    $sudo_password = $form_state['values']['sudo_password'];
  }

  vhost_create_vhost($object, $sudo_password);

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

function vhost_import_form() {

  $conf_files = array();
  $vhost_path = variable_get('vhost_sites_conf_path');
  if ($handle = opendir($vhost_path)) {
    while (false !== ($entry = readdir($handle))) {

      $file = variable_get('vhost_sites_conf_path') . '/' . $entry;
      $file = file_get_contents($file);
      $file_lines = explode("\n", $file);
      $server_name   = NULL;

      foreach ($file_lines as $line) {
        if (strstr($line, 'ServerName')) {
          $line  = trim($line);
          $line  = explode(' ', $line);
          $value = $line[1];
          if (db_query('SELECT oid FROM {vhost_objects} WHERE server_name = :value', array(':value' => $value))->fetchAssoc()) {
            continue 2;
          }
          continue;
        }
      }

      if (substr($entry, 0, 1) != '.') {
        $conf_files[$entry] = $entry;
      }
    }
    closedir($handle);
  }

  natsort($conf_files);

  $form['conf_files'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Untracked hosts'),
    '#options' => $conf_files,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Importort Selections'),
  );

  return $form;
}

function vhost_import_form_submit($form, &$form_state) {
  $conf_files = $form_state['input']['conf_files'];
  $document_root_strlen = (strlen(variable_get('vhost_document_root_prefix')) + 1);

  foreach ($conf_files as $key => $value) {
    if (!$value) continue;

    $file = variable_get('vhost_sites_conf_path') . '/' . $key;
    $file = file_get_contents($file);
    $file_lines = explode("\n", $file);

    $document_root = NULL;
    $server_name   = NULL;
    $server_alias  = array();

    foreach ($file_lines as $line) {
      $is_document_root = strstr($line, 'DocumentRoot');
      $is_server_name   = strstr($line, 'ServerName');
      $is_server_alias  = strstr($line, 'ServerAlias');

      if ($is_document_root || $is_server_name || $is_server_alias) {
        $line  = trim($line);
        $line  = explode(' ', $line);
        $value = $line[1];

        switch ($line[0]) {
          case 'DocumentRoot':
            $document_root = substr($value, $document_root_strlen, -1);
            break;
          case 'ServerName':
            $server_name = $value;
            break;
          case 'ServerAlias':
            $server_alias[] = $value;
            break;
        }
      } else {
        continue;
      }
    }

    $object = array(
      'created' => REQUEST_TIME,
      'modified' => REQUEST_TIME,
      'document_root' => $document_root,
      'server_name' => $server_name,
      'server_alias' => implode("\n", $server_alias),
    );

    db_insert('vhost_objects')
      ->fields($object)
      ->execute();

    drupal_set_message(t('Successfully imported: !vhost', array('!vhost' => $key)));
  }
}
