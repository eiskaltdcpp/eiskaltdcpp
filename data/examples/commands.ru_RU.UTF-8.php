#!/usr/bin/php
<?php
#------------------------------------------------------------------------------#
#------------------------------------------------------------------------------#
#
# EiskaltDC++ - personal info script.
#
#------------------------------------------------------------------------------#
#
# Поддерживыемые проигрыватели:
#
# Видео: totem, smplayer, vlc, gnome-mplayer, xine
# Аудио: rhythmbox, audacious2, amarok
#
#------------------------------------------------------------------------------#
# Author:  GoTLiuM InSPiRiT <gotlium@gmail.com>
# Date:    08.06.2010
# License: GPL-2 (http://opensource.org/licenses/gpl-2.0.php)
#------------------------------------------------------------------------------#
#------------------------------------------------------------------------------#


/**
* Примеры использования
*
* Установка в Debian и Ubuntu (если php еще не был установлен):
* $ sudo apt-get install php5-cli
*
* Добавьте псевдонимы (команды в чате):
* /alias p::/sh  /usr/share/eiskaltdcpp/examples/commands.ru_RU.UTF-8.php
* /alias u::/sh  /usr/share/eiskaltdcpp/examples/commands.ru_RU.UTF-8.php uptime
* /alias b::/sh  /usr/share/eiskaltdcpp/examples/commands.ru_RU.UTF-8.php black
* /alias r::/sh  /usr/share/eiskaltdcpp/examples/commands.ru_RU.UTF-8.php ratio
* /alias t::/sh  /usr/share/eiskaltdcpp/examples/commands.ru_RU.UTF-8.php torrent
* /alias ps::/sh /usr/share/eiskaltdcpp/examples/commands.ru_RU.UTF-8.php process
* /alias pn::/sh /usr/share/eiskaltdcpp/examples/commands.ru_RU.UTF-8.php ping
* /alias ip::/sh /usr/share/eiskaltdcpp/examples/commands.ru_RU.UTF-8.php ip
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
*
* Проверить список доступных псевдонимов можно так:
* /alias list
**/


final class EiskaltDC
{
  // имя системного пользователя (заполнять не требуется)
  private $usename = '';

  // смайлики, которые используем (здесь можно указать коды любимых смайлов)
  private $smiles = array(
    ':дэнс:', ':diablored:', ':diabloblack:', ':Кенни:',
    ':нинзя:', ':бубльгум:', '}:-Df>', ':смерть:',
    ':роджер:', ':рокер:', ':гитарист:'
  );

  // переменная содержащая ошибки (заполнять не требуется)
  public  $errors  = array();

/**
   * Метод по умолчанию
   *
   * @access public
   * @return void
   */
  public function __construct()
  {
    if( version_compare(PHP_VERSION, '5.2.0', '<') ) {
      $this->errors[] = null;
    }
    elseif( function_exists('exec') && stristr(PHP_OS, 'lin') )
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
   * То что смотрим в видео проигрывателе или слушаем в аудио
   *
   * @access public
   * @return string
   */
  public function play()
  {
    // имена процессов видео-проигрывателей
    $vplayers = array('totem', 'smplayer', 'vlc', 'gnome-mplayer', 'xine');

    // bash команды, для извлечения заголовков
    $get_title[] = 'id=`xdotool search --class totem | head -n 1` && xprop -id $id | grep "WM_ICON_NAME" | head -n 1 | sed -n \'s/.*"\(.*\)\"/\1/p\'';
    $get_title[] = 'id=`xdotool search --class smplayer | head -n 1` && xprop -id $id | grep "WM_NAME(STRING)" | head -n 1 | sed -n \'s/.*"\(.*\)\s-\sSMPlayer"/\1/p\'';
    $get_title[] = 'id=`xdotool search --class vlc | head -n 1` && xprop -id $id | grep "WM_NAME(COMPOUND_TEXT)" | head -n 1 | sed -n \'s/.*"\(.*\)\-\(.*\)\-\(.*\)VLC\"/\1/p\'';
    $get_title[] = 'id=`xdotool search --class gnome-mplayer | head -n 1` && xprop -id $id | grep "WM_ICON_NAME" | head -n 1 | sed -n \'s/.*"\(.*\)\s-\sGNOME\(.*\)\"/\1/p\'';
    $get_title[] = 'id=`xdotool search --class xine | head -n 1` && xprop -id $id | grep "WM_ICON_NAME" | head -n 1 | sed -n \'s/.*"\(.*\)\s-\s\(.*\)\"/\2/p\'';

    // проверка наличия процессов. если имеется хоть один,
    // то извлекается заголовок и соответственно в чат.
    foreach( $vplayers as $id => $player )
    {
      exec("ps -e | grep $player | awk '{print $1}'", $ps_tm);

      if( is_array($ps_tm) && count($ps_tm) == 1 && is_numeric($ps_tm[0]) )
      {
        exec($get_title[$id], $win_title);

        if( is_array($win_title) && count($win_title) == 1 && !empty($win_title[0]) )
        {
          if( preg_match('#[a-zа-я]#iu',$win_title[0]) )
          {
            return 'смотрю что-то под названием: '.trim($win_title[0]).' :ТВ:';
          }
        }

        unset($win_title);
      }

      unset($ps_tm);
    }

    unset( $id, $player );

    // Уровень системного звука
    exec("amixer sget Master | grep \"Mono:\" | awk '{print $4}'",$sysvol);

    if( empty($sysvol[0]) ) {
      $sysvol = array();
      exec("amixer sget Master | grep \"Front Left:\" | awk '{print $5}'",$sysvol);
    }

    if( is_array($sysvol) && !empty($sysvol[0]) ) {
      preg_replace('#\[(.*)\]#e', '$sys_volume="$1";', $sysvol[0]);
    }

    unset($sysvol);
    //


    // имена процессов аудио-проигрывателей
    $aplayers = array('rhythmbox', 'audacious2', 'amarok');

    // bash команды
    $get_data[] = 'echo `rhythmbox-client --print-playing-format %aa,%at,%ay,%tt,%td`';
    $get_data[] = 'audacious=`php -r \'exec("audtool2 current-song && audtool2 current-song-bitrate-kbps",$aud);$tg="/usr/share/php-getid3/getid3.php";$f=false;if(file_exists($tg)){include $tg;exec("audtool2 current-song-filename",$file);$getID3 = new getID3;$info = $getID3->analyze($file[0]);if(isset($info["id3v1"])){print $info["id3v1"]["artist"].",".$info["id3v1"]["album"].",".$info["id3v1"]["year"].",".$info["id3v1"]["title"].",,{$aud[1]}";}else{$f=true;}}else{$f=true;}if($f==true){print",,,{$aud[0]},,{$aud[1]}";}\'` && echo $audacious';
    $get_data[] = 'amarock=`qdbus org.kde.amarok /Player org.freedesktop.MediaPlayer.GetMetadata 2>/dev/null` && artist=`echo "$amarock" | sed -ne \'s/^artist: \(.*\)$/\1/p\'` && album=`echo "$amarock" | sed -ne \'s/^album: \(.*\)$/\1/p\'` && year=`echo "$amarock" | sed -ne \'s/^year: \(.*\)$/\1/p\'` && title=`echo "$amarock" | sed -ne \'s/^title: \(.*\)$/\1/p\'` && bitrate=`echo "$amarock" | sed -ne \'s/^audio\-bitrate: \(.*\)$/\1/p\'` && echo "$artist,$album,$year,$title,,$bitrate"';

    // используемые поля
    $info_head = array('группой','альбом','год','песня','время','bitrate');

    foreach( $aplayers as $id => $player )
    {
      exec("ps -e | grep $player | awk '{print $1}'", $ps);

      if( is_array($ps) && count($ps) == 1 && is_numeric($ps[0]) )
      {
        exec($get_data[$id], $win_title);

        if( is_array($win_title) && !empty($win_title[0]) )
        {
          $info = explode(',', $win_title[0]);
          $out = array();

          if( is_array($info) && count($info) > 0 ):

          foreach( $info as $i => $value )
          {
            $value = trim($value);

            if( $value != 'Неизвестно' && $value != 'Unknown' && $value != '0' && !empty($value) && preg_match('#[a-zа-я0-9]#iu',$value) )
            {
              $out[] = "{$info_head[$i]}: $value";
            }
          }

          if( is_array($out) && count($out) > 1 )
          {
            if( $player == 'rhythmbox' ):
            exec("rhythmbox-client --print-volume | awk '{print $5}'", $vol);
            $volume = substr($vol[0],0,(strlen($vol[0])-1));
            $volume = str_replace(',','.',$volume);
            $volume = $volume * 100;
            $volume = number_format($volume);
            endif;

            shuffle($this->smiles);
            $format = implode(', ', $out);
            $volume = (!empty($volume)) ? 'уровень громкости: (player:'.$volume.'% / system:'.$sys_volume.')': "уровень сис. громкости: $sys_volume";

            return 'наслаждаюсь '.implode(', ', $out).", $volume " . current($this->smiles);
          }

          endif;

        }

        unset($win_title, $out);
      }

      unset($ps);
    }

    return 'enjoy the silence... :слушаю:';
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
   * IP адрес интерфейса ppp0 или если прокинули
   * порты, с checkip сервиса
   *
   * @access public
   * @return string
   */
  public function ip()
  {
    exec('ifconfig | grep -A 1 ppp0 | grep inet | cut -d ":" -f2 | cut -d " " -f1', $ip);

    if( is_array($ip) && count($ip) == 1 ) {
      return "my ip: {$ip[0]} d'dos-те меня, мне всё равно! :ПиСи:";
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

if( count($eiskalt->errors) > 0 ) {
  print "php v.5.2 <, foo 'exec' not exists or os is not linux!";
}
elseif( method_exists($eiskalt, $method) ) {
  $str = $eiskalt->$method();
  print (!count($eiskalt->errors)) ? "+me -> $str\n" : "error result\n";
} else {
  print "Error usage!\n";
}
?>
