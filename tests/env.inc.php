<?php

define('FEATURE_DISABLE_ZRUNTIME',true);

ob_start();
require_once(dirname(__FILE__)."/game_env.inc.php");
ob_end_clean();
