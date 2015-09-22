<?php if ($object['support_ssl']): ?>

<VirtualHost *:<?php echo variable_get('vhost_ssl_port', '443'); ?>>
  SSLEngine on

  SSLCertificateFile <?php echo $object['ssl_cert_path'] ? $object['ssl_cert_path'] : variable_get('vhost_ssl_cert_path', NULL); ?>

  SSLCertificateKeyFile <?php echo $object['ssl_key_path'] ? $object['ssl_key_path'] : variable_get('vhost_ssl_key_path', NULL); ?>


  DocumentRoot "<?php echo $object['document_root'] ?>"
  ServerName <?php echo $object['server_name'] ?>

  <?php echo $object['server_alias'] ?>

</VirtualHost>

<?php endif; ?>

<VirtualHost *:<?php echo variable_get('vhost_port', '80'); ?>>
  DocumentRoot "<?php echo $object['document_root'] ?>"
  ServerName <?php echo $object['server_name'] ?>

  <?php echo $object['server_alias'] ?>

  <?php if (variable_get('vhost_custom_logs', FALSE)): ?>
    ErrorLog "<?php echo variable_get('vhost_log_path') . $object['server_name'] ?>/error_log"
    CustomLog "<?php echo variable_get('vhost_log_path') . $object['server_name'] ?>/access_log" common
  <?php endif; ?>

</VirtualHost>

<Directory "<?php echo $object['document_root']; ?>">
    Options Indexes FollowSymLinks
    AllowOverride All
    Require all granted
</Directory>
