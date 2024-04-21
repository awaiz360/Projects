<!DOCTYPE html>    
<html>    
<head>    
    <title>BloodStream Web Portal</title>    
    <link rel="stylesheet" type="text/css" href="style.css">    
    <link rel="icon" type="image/x-icon" href="favicon.jpeg">
</head>    
<body>    
    <h2>BloodStream<br>Admin Login Page</h2><br>    
    <div class="login">    
    <form id="login" method="post" action="<?php echo htmlspecialchars($_SERVER['PHP_SELF']);?>">    
        <label><b>User Name     
        </b>    
        </label>    
        <input type="text" name="Uname" id="Uname" placeholder="Username" required="true">    
        <br><br>    
        <label><b>Password     
        </b>    
        </label>    
        <input type="Password" name="Pass" id="Pass" placeholder="Password" required="true">    
        <br><br>    
        <input type="submit" name="log" id="log" value="Log In">       
        <br><br>    
        <input type="checkbox" id="check">    
        <span>Remember me</span>    
        <br><br>    
        <a href="#" style="float: right;  
    background-color: none;
    color: #e21d24;">Forgot Password</a>    
    </form>     
    <?php
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $uname = $_POST['Uname'];
    $upass = $_POST['Pass'];
    if ($uname == 'admin' && $upass == 'blood123') {
        header("Location: info.php");
        exit();
    } else {
        echo "Incorrect Username or Password !!";
    }
}
?>
</div>    
</body>    
</html>    