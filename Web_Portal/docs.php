<!DOCTYPE html>    
<html>    
<head>    
    <title>BloodStream Web Portal</title>    
    <link rel="stylesheet" type="text/css" href="style.css">    
    <link rel="icon" type="image/x-icon" href="favicon.jpeg">
</head>    
<body>    
    <h2>BloodStream<br>Verify Documents</h2><br>    
    <div class="login" style="text-align: center; padding: 10px;">    
    <form id="docs" method="post" action="<?php echo htmlspecialchars($_SERVER['PHP_SELF']);?>">    
        <label><b>Patient Name     
        </b><br><br>    
        </label>    
        <input type="file" name="docVer" id="docVer" required="true">    
        <br><br>        
        <input type="Submit" name="log" id="log" value="Save Image">       
        <br><br>    
    </form>     
    <?php

?>
</div>    
</body>    
</html>    