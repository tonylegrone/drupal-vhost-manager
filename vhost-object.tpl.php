
<VirtualHost *:<?php echo variable_get('vhost_port', '80'); ?>>
  DocumentRoot "<?php echo $object['document_root'] ?>"
  ServerName <?php echo $object['server_name'] ?>

  <?php echo $object['server_alias'] ?>

  ErrorLog "<?php echo variable_get('vhost_log_path') . $object['server_name'] ?>/error_log"
  CustomLog "<?php echo variable_get('vhost_log_path') . $object['server_name'] ?>/access_log" common
</VirtualHost>

<Directory "<?php echo $object['document_root']; ?>">
    Options Indexes FollowSymLinks
    AllowOverride All
    Order allow,deny
    Allow from all
</Directory>
