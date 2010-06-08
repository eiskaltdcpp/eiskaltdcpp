#!/usr/bin/php5
<?php
#--------------------------------------------------#
#--------------------------------------------------#
#
# EiskaltDC++ - personal info script.
#
#--------------------------------------------------#
# Author:  GoTLiuM InSPiRiT <gotlium@gmail.com>
# Date:    28.05.2010
# License: GPL (http://www.opensource.org/licenses/gpl-license.php)
#--------------------------------------------------#
#--------------------------------------------------#


/**
* Примеры использования
*
* Добавьте псевдонимы:
* /alias p::/sh  /usr/share/eiskaltdcpp/commands.ru_RU.UTF-8.php
* /alias u::/sh  /usr/share/eiskaltdcpp/commands.ru_RU.UTF-8.php uptime
* /alias b::/sh  /usr/share/eiskaltdcpp/commands.ru_RU.UTF-8.php black
* /alias r::/sh  /usr/share/eiskaltdcpp/commands.ru_RU.UTF-8.php ratio
* /alias ps::/sh /usr/share/eiskaltdcpp/commands.ru_RU.UTF-8.php process
* /alias t::/sh  /usr/share/eiskaltdcpp/commands.ru_RU.UTF-8.php torrent
* /alias pn::/sh /usr/share/eiskaltdcpp/commands.ru_RU.UTF-8.php ping
* /alias ip::/sh /usr/share/eiskaltdcpp/commands.ru_RU.UTF-8.php ip
*
* Теперь в чате будут доступны команды:
* /p  - то что играет в rhythmbox или в totem
* /u  - ваш uptime
* /b  - ваш черный список
* /r  - ваше соотношение
* /t  - ваше соотношение в торрент клиенте
* /ps - системная информация
* /pn - пингуем сайт mail.ru
* /ip - ваш ip адрес
**/


final class EiskaltDC
{
  // имя системного пользователя
  private $usename = '';

  // смайлики, которые используем
  private $smiles = array(
    ':дэнс:', ':diablored:', ':diabloblack:', ':Кенни:',
    ':нинзя:', ':бубльгум:', '}:-Df>', ':смерть:',
    ':роджер:', ':рокер:', ':гитарист:'
  );

  // переменная содержащая ошибки
  public  $errors  = array();

  /**
   * Метод по умолчанию
   *
   * @access public
   * @return void
   */
  public function __construct()
  {
    if( function_exists('exec') && stristr(PHP_OS, 'lin') )
    {
      exec('whoami', $username);

      if( is_array($username) && count($username) == 1 )
      {
        $this->username = trim(end($username));
      }
      else {
        $this->errors[] = null;
      }
    }
    else {
      $this->errors[] = null;
    }
  }




  /**
   * Метод для форматирования байтов
   *
   * @access private
   * @param int    $bytes
   * @param int    $precision
   * @param string $space
   * @return int
   */
  private function formatBytes($bytes, $precision = 2, $space = ' ' )
  {
    $units = array('B', 'KB', 'MB', 'GB', 'TB');

    $bytes = max($bytes, 0);
    $pow   = floor(($bytes ? log($bytes) : 0) / log(1024));
    $pow   = min($pow, count($units) - 1);

    $bytes /= pow(1024, $pow);

    return round($bytes, $precision) . $space . $units[$pow];
  }




  /**
   * Метод для вывода соотношения dcpp клиента
   *
   * @access public
   * @return string
   */
  public function ratio()
  {
    $filename = '/home/'.$this->username.'/.eiskaltdc++/EiskaltDC++.xml';

    if( file_exists( $filename ) )
    {
      $file = file_get_contents( $filename );
      exec('ps -o pcpu,time,rss -C eiskaltdcpp | tail -1 | awk \'{print $1,$2,$3}\'', $info);

      if( is_array($info) && !empty($info[0]) ) {
        list($cpu,$time,$mem) = explode(" ", $info[0]);
        $mem = self::formatBytes( ($mem*1024) );
      } else {
        $cpu = '';$time = '';$mem = '';
      }

      preg_replace('#<app-stat-total-down type="string">(.*?)</app-stat-total-down>#e', '$total_down="$1";', $file);
      preg_replace('#<app-stat-total-up type="string">(.*?)</app-stat-total-up>#e', '$total_up="$1";', $file);

      if( is_numeric($total_down) && is_numeric($total_up) )
      {
        $ratio = $total_up / $total_down;
        $ratio = number_format($ratio,2);

        $total_down = self::formatBytes($total_down);
        $total_up   = self::formatBytes($total_up);

        return "dcpp-client ratio: $ratio (отдачи: $total_up / загрузки: $total_down), sys: (cpu:$cpu%, time:$time, mem:$mem) :зараза:";
      }
      else {
        $this->errors[] = null;
      }
    } else {
      return "dcpp-client: пока ничего не качал :-)";
    }
  }




  /**
   * То что смотрим в Totem или слушаем в Rhythmbox
   *
   * @access public
   * @return string
   */
  public function play()
  {
    exec("ps -e | grep totem | awk '{print $1}'", $ps_tm);

    if( is_array($ps_tm) && count($ps_tm) == 1 && is_numeric($ps_tm[0]) )
    {
      exec('id=`xdotool search --class totem | head -n 1` && xprop -id $id | grep "WM_ICON_NAME" | head -n 1', $win_title);

      if( is_array($win_title) && count($win_title) == 1 && !empty($win_title) )
      {
        preg_replace('#\=[\s]"(.*?)"#e', '$title="$1";', $win_title[0]);

        if(!empty($title)) {
          return 'смотрю что-то под названием: '.str_replace('"', '', $title) . ' :ТВ:';
        }
      }
    }

    exec("ps -e | grep rhythmbox | awk '{print $1}'", $ps);

    if( is_array($ps) && count($ps) == 1 && is_numeric($ps[0]) )
    {

      exec("rhythmbox-client --print-volume | awk '{print $5}'", $vol);

      $volume = substr($vol[0],0,(strlen($vol[0])-1));
      $volume = str_replace(',','.',$volume);
      $volume = $volume * 100;
      $volume = number_format($volume);

      exec("echo `rhythmbox-client --print-playing-format \"%aa,%at,%ay,%tt,%td\"`",$info);
      exec("amixer sget Master | grep \"Mono:\" | awk '{print $4}'",$sysvol);
      if( empty($sysvol[0]) ) {
        $sysvol = array();
        exec("amixer sget Master | grep \"Front Left:\" | awk '{print $5}'",$sysvol);
      }
      preg_replace('#\[(.*)\]#e', '$sysvol="$1";', $sysvol[0]);

      $info = explode(',', $info[0]);
      $info_head = array('группой','альбом','год','песня','время');
      $out = array();

      foreach( $info as $i => $value )
      {
        $value = trim($value);

        if( $value != 'Неизвестно' && $value != 'Unknown' && $value != '0' && !empty($value) && preg_match('#[a-zа-я]#iu',$value) )
        {
          $out[] = "{$info_head[$i]}: $value";
        }
      }

      if( is_array($out) && count($out) > 1 )
      {
        shuffle($this->smiles);
        $format = implode(', ', $out);
        return 'наслаждаюсь '.implode(', ', $out).', уровень громкости: (rhythmbox:'.$volume.'% / system:'.$sysvol.') ' . current($this->smiles);
      }
      else {
        return 'enjoy the silence... :слушаю:';
      }
    }
    else {
      return 'enjoy the silence... :слушаю:';
    }
  }




  /**
   * Uptime вашей машины
   *
   * @access public
   * @return string
   */
  public function uptime()
  {
    exec('uptime', $uptime);

    if( is_array($uptime) && !empty($uptime[0]) ) {
      return "uptime: {$uptime[0]} :радиация:";
    }
    else {
      $this->errors[] = null;
      return false;
    }
  }




  /**
   * Black List EiskaltDC++
   *
   * @access public
   * @return string
   */
  public function black()
  {
    $filename = "/home/{$this->username}/.eiskaltdc++/blacklist";

    if( file_exists($filename) ) {
      $black = file($filename);
      $black = implode(", ", $black);
      return "black list: вы должны знать мозготрахов в лицо: ".str_replace("\n","",$black);
    } else {
      return "black list: чёрный список пока пуст!";
    }
  }




  /**
   * Касательно всё системы
   *
   * @access public
   * @return string
   */
  public function process()
  {
    exec('ps -e | wc -l', $process);
    exec('free -t -b | grep Total: | awk \'{print $2,$3,$4}\'', $memory);
    exec('netstat -t -n | wc -l', $tcp);
    exec('netstat -u -n | wc -l', $udp);
    exec('ps -eo pcpu | sort -k 1 -r | awk \'{print $1}\'', $cpu);
    exec('df -l -h --total | tail -1 | awk \'{print $2,$3,$4}\'', $space);
    exec('uname', $os);


    $udp = (empty($udp[0]) or !@count($udp)) ? 0 : $udp[0];
    $tcp = (empty($tcp[0]) or !@count($udp)) ? 0 : $tcp[0];
    $cpu = (empty($tcp[0]) or !@count($udp)) ? 0 : array_sum($cpu);
    $os  = (empty($os[0])  or !@count($os)) ? 'Ubuntu' : $os[0];

    $space = explode(" ", $space[0]);

    $memory = explode(' ',$memory[0]);
    $memory[0] = self::formatBytes($memory[0],2,'');
    $memory[1] = self::formatBytes($memory[1],2,'');
    $memory[2] = self::formatBytes($memory[2],2,'');

    if( is_array($process) && count($process) == 1 && is_numeric($process[0]) ) {
      return "SysInfo: os: $os, cpu: $cpu%, proc: {$process[0]}, mem+swap: (t:{$memory[0]},u:{$memory[1]},f:{$memory[2]}), hdd: (t:{$space[0]},u:{$space[1]},f:{$space[2]}), sockets: (tcp:$tcp/udp:$udp) :!!!:";
    } else {
      $this->errors[] = null;
    }
  }




  /**
   * BitTorrent Transmission
   *
   * @access public
   * @return string
   */
  public function torrent()
  {
    $filename = '/home/'.$this->username.'/.config/transmission/stats.json';

    if( file_exists($filename) )
    {
      $file  = file_get_contents($filename);
      $stats = json_decode($file, true);

      exec('ps -o pcpu,time,rss -C transmission | tail -1 | awk \'{print $1,$2,$3}\'', $info);

      if( is_array($info) && !empty($info[0]) ) {
        list($cpu,$time,$mem) = explode(" ", $info[0]);
        $mem = self::formatBytes(($mem*1024));
      } else {
        $cpu = '';$time = '';$mem = '';
      }

      if( is_numeric($stats['downloaded-bytes']) && is_numeric($stats['uploaded-bytes']) )
      {
        $ratio = $stats['uploaded-bytes'] / $stats['downloaded-bytes'];
        $ratio = number_format($ratio,2);

        $total_down = self::formatBytes($stats['downloaded-bytes']);
        $total_up   = self::formatBytes($stats['uploaded-bytes']);

        return "torrent-client ratio: $ratio (отдачи: $total_up / загрузки: $total_down), sys: (cpu: $cpu%, time: $time, mem: $mem) :качаю:";
      }
      else {
        $this->errors[] = null;
      }
    }
    else
    {
      return 'torrent-client: пока ничего не качал :-)';
    }
  }




  /**
   * Пингуем Mail.ru
   *
   * @access public
   * @return string
   */
  public function ping()
  {
    exec('ping mail.ru -c 4', $ping);
    if( is_array($ping) && count($ping) > 1 ) {
      return "Ping Check :ПиСи:\n".implode("\n",$ping);
    } else {
      $this->errors[] = null;
    }
  }




  /**
   * IP адрес интерфейса ppp0
   *
   * @access public
   * @return string
   */
  public function ip()
  {
    exec('ifconfig | grep -A 1 ppp0 | grep inet | cut -d ":" -f2 | cut -d " " -f1', $ip);

    if( is_array($ip) && count($ip) == 1 ) {
      return "my ip: {$ip[0]} d'dos-те меня, мне пох! :ПиСи:";
    } else {
      $f=@fopen('http://checkip.dyndns.com/','rb');
      if( $f ) {
        $text='';
        while(!feof($f)) $text.=fread($f,1024);
        fclose($f);
        if(!empty($text)) {
          preg_replace('#<body>Current[\s]IP[\s]Address:[\s](.*)</body>#e','$chip="$1";', $text);
        }
      }
      if( !empty($chip) && preg_match('#^(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})$#',trim($chip)) ) {
        return "my ip: $chip d'dos-те меня, мне всё равно! :ПиСи:";
      }
      else {
        $this->errors[] = null;
        return false;
      }
    }
  }

}

$method  = (isset($argv[1])) ? trim($argv[1]) : 'play';
$eiskalt = new EiskaltDC;

if( !count($eiskalt->errors) && method_exists($eiskalt, $method) ) {
  $str = $eiskalt->$method();
  print (!count($eiskalt->errors)) ? "+me -> $str\n" : "error result\n";
} else {
  print "Error usage!\n";
}
?>
