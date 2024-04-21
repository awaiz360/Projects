<!DOCTYPE html>    
<html>    
<head>    
    <title>BloodStream Web Portal</title>    
    <link rel="stylesheet" type="text/css" href="style.css">    
    <link rel="icon" type="image/x-icon" href="favicon.jpeg">
</head>    
<body>    
    <h2>BloodStream<br>Add New Patient</h2><br>    
    <div class="login" style="text-align: center; padding: 10px;">    
    <form id="login" method="post" action="<?php echo htmlspecialchars($_SERVER['PHP_SELF']);?>">    
        <label><b>Patient Name     
        </b><br>    
        </label>    
        <input type="text" name="Pname" id="Pname" placeholder="Name" required="true">    
        <br><br>    
        <label><b>Blood Group     
        </b><br>    
        </label>    
        <input type="text" name="Bgroup" id="Bgroup" placeholder="Blood Type" required="true">    
        <br><br>
        <label><b>Location     
        </b><br>    
        </label>    
        <input type="text" name="loc" id="loc" placeholder="Location" required="true">    
        <br><br>
        <label><b>Mobile Number     
        </b><br>    
        </label>    
        <input type="number" name="Pnum" id="Pnum" placeholder="Contact No." required="true">    
        <br><br>
        <label><b>Registration Date     
        </b><br>    
        </label>    
        <input type="date" name="Rdate" id="Rdate" placeholder="Date" required="true">    
        <br><br>    
        <input type="submit" name="log" id="log" value="Add Patient">       
        <br><br>    
    </form>     
    <?php

?>
</div>    
</body>    
</html>    