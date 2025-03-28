<?php // -*-php-*-

$WEBSITE_TITLE = 'OPUS Explorer';


function check_setup(){
    global $USER_DATADIR,$USER_DB;
    if (!file_exists($USER_DATADIR)){
        echo("User data dir does not exist!<br/>");
        exit;
    }
    if (!file_exists($USER_DB)){
        if (! create_user_db($USER_DB)){            
            echo("Cannot create user database!<br/>");
            exit;
        }
    }
}

function logged_in ( ) {

    // already logged in! --> return true
    if (isset($_SESSION['user'])){
        return true;
    }

    $PHP_SELF = $_SERVER['PHP_SELF'];
        
    global $ALLOW_GUEST_USERS;
    
    if ($ALLOW_GUEST_USERS){
        if (isset($_POST['user']) && isset($_POST['password'])){
            if ($_POST['user'] == 'guest' && $_POST['password'] == 'guest'){
                $_SESSION['user'] = 'guest';
                $_SESSION['email'] = 'anonymous';
                return true;
            }
        }
    }

    global $ALLOW_NEW_USERS;
    global $USER_DB;

    check_setup();    
    if (isset($_POST['reset'])){
        if ($_POST[email]){
            if (prepare_password_reset($USER_DB, $_POST['email'])){
                echo("Message sent to reset your password, please, check your e-mail!");
            }
            else{
                echo('Failed to send reset message. Please, contact opus-project AT helsinki.fi!');
            }
        }
        else{
            echo('Specify a valid e-mail address to send a reset message!');
        }
    }
    elseif ($_POST['user'] && $_POST['password']){
        if (verify_password($USER_DB, $_POST['user'], $_POST['email'], $_POST['password'])){
            $_SESSION['user'] = $_POST['user'];
            $_SESSION['email'] = $_POST['email'];
            return true;
        }
        elseif ($_GET['reset']){
            if (renew_password($USER_DB, $_POST['user'], $_POST['email'], $_POST['password'], $_GET['reset'])){
                echo "Successfully changed password for user ".$_POST['user']."!<br/><br/>";
                $_SESSION['user'] = $_POST['user'];
                $_SESSION['email'] = $_POST['email'];
                return true;        
            }
        }
        elseif ($ALLOW_NEW_USERS){
            if (add_user($USER_DB,$_POST['user'],$_POST['password'],$_POST['email'])){
                $_SESSION['user'] = $_POST['user'];
                $_SESSION['email'] = $_POST['email'];
                return true;
            }
        }
        else{
            echo "Login failed! Try again!";
        }
    }
    elseif (isset($_GET['verify'])){
        if (verify_user($USER_DB, $_GET['email'], $_GET['verify'])){
            echo "Successfully verified account for ".$_GET['email']."!<br/>";
            echo "Please, login with your username and password below!<br/>";
        }
    }
    elseif (isset($_GET['reset']) && isset($_GET['email'])){
        if ($_GET['reset'] && $_GET['email']){
            echo('<div class="rightalign"><a href="help.php">[help]</a>[login]</div></br><hr>');
            echo "Password reset requested: Change your password below!<br/><br/>";
            echo "<form action=\"$PHP_SELF\" method=\"post\"><table class='login'>";
            echo '<tr><td class="login">username:</td><td class="login"><input type="user" value="'.$_GET['user'].'" name="user"></td></tr>';
            echo '<tr><td class="login">  e-mail:</td><td class="login"><input type="email" value="'.$_GET['email'].'" name="email"></td></tr>';
            echo '<tr><td class="login">password:</td><td class="login"><input type="password" name="password"></td></tr>';
            echo '</table><p>';
            echo '<input type="submit" name="submit" value="submit">';
            echo '</form>';
            return false;
        }
    }

    echo('<div class="rightalign"><a href="help.php">[help]</a>[login]</div></br><hr>');
    echo "<br /><br /><h2>Login</h2>";

    if ($ALLOW_NEW_USERS){
        echo "Login or create a new user using the form below!<br/>";
        echo "Allowed characters for your username: a-zA-Z0-9_<br />";
    }
    if ($ALLOW_GUEST_USERS){
        echo "There is a guest account for testing: use username = guest, password = guest<br />";
    }

    echo "<br/><form action=\"$PHP_SELF\" method=\"post\"><table class='login'>";
    echo '<tr><td class="login">username:</td><td class="login"><input type="user" name="user"></td></tr>';
    echo '<tr><td class="login">  e-mail:</td><td class="login"><input type="email" name="email"></td></tr>';
    echo '<tr><td class="login">password:</td><td class="login"><input type="password" name="password"></td></tr>';
    echo '</table><p>';
    echo '<input type="submit" name="submit" value="login">';
    echo '<input type="submit" name="reset" value="reset password"></p>';
    echo '</form>';

    return false;

}



function prepare_password_reset($user_db, $email){
    global $WEBSITE_TITLE;
    if ($dbh = new PDO('sqlite:'.$user_db)){
        if (! $sth = $dbh->prepare('SELECT name FROM users WHERE email = ?')){
            echo "\nPDO::errorInfo():\n";
            print_r($dbh->errorInfo());
            return false;
        }
        $sth->execute(array($email));
        if (!$result = $sth->fetch(PDO::FETCH_ASSOC)){
            echo "User not found for e-mail ".$email.'<br/>';
            return false;
        }
        $user = $result['name'];
        $secret = password_hash($email, PASSWORD_DEFAULT);
        $sth = $dbh->prepare('UPDATE users SET status = ?, secret = ? WHERE email = ?');
        if (! $sth->execute(array('reset', $secret, $email))) return false;
    }


    $to = $email;
    $subject = $WEBSITE_TITLE.'- Reset Password';
    $headers  = 'MIME-Version: 1.0' . "\n";
    $headers .= 'Content-type: text/html; charset=iso-8859-1' . "\n";
    /*
    $from = 'opus-project@helsinki.com';
    $headers .= 'From: '.$from."\n".
             'Reply-To: '.$from."\n" .
             'X-Mailer: PHP/' . phpversion();
    */

    $param = make_query(['user' => $user, 'email' => $email, 'reset' => $secret]);
    $url = url().'?'.$param;

    $message = '<html><body><h2>OPUS-MT dashboard: Password reset request</h2>';
    $message .= '<p>Your user name is: '.$user.'</p>';
    $message .= '<p><a href="'.$url.'">Click on this links to reset your password</a> ';
    $message .= "or open the following URL in your browser:<br/><br/>".$url;
    $message .= '</p></body></html>';
                
    return mail($to, $subject, $message, $headers);
}




function create_user_db($user_db){
    if ($dbh = new PDO('sqlite:'.$user_db)){
        $sql = $dbh->prepare('CREATE TABLE users (name TEXT NOT NULL UNIQUE, email TEXT PRIMARY KEY UNIQUE, password TEXT NOT NULL, status TEXT NOT NULL, secret TEXT)');
        return $sql->execute();
    }
    return false;
}


function renew_password($user_db, $user, $email, $password, $secret){
    if (!$user){
        echo "No username given! Try again!<br/>";
        return false;
    }
    if (!$password){
        echo "No password given! Try again!<br/>";
        return false;
    }
    if ($dbh = new PDO('sqlite:'.$user_db)){
        $sth = $dbh->prepare('SELECT secret FROM users WHERE name = ? AND email = ?');
        $sth->execute(array($user, $email));
        if (!$result = $sth->fetch(PDO::FETCH_ASSOC)){
            echo "User not found for e-mail ".$email.' and user name '.$user.'<br/>';
            return false;
        }
        if ($secret === $result['secret']){
            $passkey = password_hash($password, PASSWORD_DEFAULT);
            $sth = $dbh->prepare('UPDATE users SET password = ? WHERE name = ? AND email = ?');
            return $sth->execute(array($passkey, $user, $email));
        }
    }
    return false;
}

function add_user($user_db,$user,$password, $email){

    if (!$user){
        echo "No username given! Try again!<br/>";
        return false;
    }
    if (!$password){
        echo "No password given! Try again!<br/>";
        return false;
    }
    if ($user == 'guest'){
        echo "Username 'guest' is not allowed! Try again!<br/>";
        return false;
    }
    if (!preg_match('/^[a-zA-Z0-9_]+$/',$user)){
        echo "Please, use only characters in the range of [a-zA-Z0-9_] for your user name<br/>";
        return false;
    }

    if ($dbh = new PDO('sqlite:'.$user_db)){

        $sth = $dbh->prepare('SELECT * FROM users WHERE email = ?');
        $sth->execute(array($email));
        if ($result = $sth->fetch(PDO::FETCH_ASSOC)){
            echo "E-mail ".$email." is already in use! ";
            echo "Login with the associated user name or try another address!<br/>";
            return false;
        }
        $sth = $dbh->prepare('SELECT * FROM users WHERE name = ?');
        $sth->execute(array($user));
        if ($result = $sth->fetch(PDO::FETCH_ASSOC)){
            echo "User name ".$user." is already in use! ";
            echo "Please, try another name!<br/>";
            return false;
        }

        $passkey = password_hash($password, PASSWORD_DEFAULT);
        $userkey = password_hash($user, PASSWORD_DEFAULT);

        $sth = $dbh->prepare("INSERT INTO users (name, email, password, status, secret) VALUES (?,?,?,?,?)");
        if ($sth){
            if ($sth->execute(array($user, $email, $passkey, 'unverified', $userkey))){
                if (send_verification_message($user, $email, $userkey)){
                    echo "Successfully created account for ".$user."!<br/>";
                    echo "Please, verify your account using the instruction in the verification message ";
                    echo "sent to you via e-mail! Thank you!<br/>";
                }
            }
        }
    }
    // echo "Failed to open user database! Contact the admin!";
    return false;
}


function send_verification_message($user, $email, $secret){
    global $WEBSITE_TITLE;
    $to = $email;
    $subject = $WEBSITE_TITLE.'- Verify user account';
    $headers  = 'MIME-Version: 1.0' . "\n";
    $headers .= 'Content-type: text/html; charset=iso-8859-1' . "\n";
    /*
    $from = 'opus-project@helsinki.com';
    $headers .= 'From: '.$from."\n".
             'Reply-To: '.$from."\n" .
             'X-Mailer: PHP/' . phpversion();
    */

    $param = make_query(['email' => $email, 'verify' => $secret]);
    $url = url().'?'.$param;

    $message = "<html><body><h2>$WEBSITE_TITLE: Verify user account</h2>";
    $message .= '<p>Your user name is: '.$user.'</p>';
    $message .= '<p><a href="'.$url.'">Click on this links to verify your account</a> ';
    $message .= "or open the following URL in your browser:<br/><br/>".$url;
    $message .= '</p></body></html>';
                
    return mail($to, $subject, $message, $headers);

}


function verify_user($user_db, $email, $secret){
    if ($dbh = new PDO('sqlite:'.$user_db)){
        $sth = $dbh->prepare('SELECT status, secret FROM users WHERE email = ?');
        $sth->execute(array($email));
        $result = $sth->fetch(PDO::FETCH_ASSOC);
        if ($result['secret'] === $secret){
            $sth = $dbh->prepare('UPDATE users SET status = "verified" WHERE email = ?');
            return $sth->execute(array($email));
        }
    }
    return false;
}

function verify_password($user_db, $user, $email, $password){
    if ($dbh = new PDO('sqlite:'.$user_db)){
        if (! $sth = $dbh->prepare('SELECT * FROM users WHERE name = ? AND email = ?')){
            echo "\nPDO::errorInfo():\n";
            print_r($dbh->errorInfo());
            return false;
        }
        if ($sth->execute(array($user, $email))){
            if ($result = $sth->fetch(PDO::FETCH_ASSOC)){
                if ($result['status'] === 'unverified'){
                    echo "The user is not yet verified! Please check your e-mail!<br/>";
                    return false;
                }
                if (password_verify($password, $result['password'])) return true;
            }
        }
    }
    return false;
}


function url(){
    if(isset($_SERVER['HTTPS'])){
        $protocol = ($_SERVER['HTTPS'] && $_SERVER['HTTPS'] != "off") ? "https" : "http";
    }
    else{
        $protocol = 'http';
    }
    return $protocol . "://" . $_SERVER['HTTP_HOST'] . $_SERVER['SCRIPT_NAME'];
}

?>
