
<VirtualHost *:80>
  DocumentRoot "<?php echo $object['document_root'] ?>"
  ServerName <?php echo $object['server_name'] ?>
  
  <?php echo $object['server_alias'] ?>
  
</VirtualHost>