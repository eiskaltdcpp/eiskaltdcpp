#!/usr/bin/php
<?php
/**
*
* @copyright (c) 2010 nE0sIghT
* @license http://opensource.org/licenses/gpl-2.0.php GNU Public License
* @version 14.03.2010
*
*/

$me = false; // Использовать +me

if($argc > 1)
	$player = $argv[1];
else
	$player = 'audacious2';

$status = 1;

switch($player)
{
	case 'xmms2':
		$player = 'XMMS2 0.5 DrLecter';

		$out = explode("\n", shell_exec('xmms2 info'));
		$data = array();
		foreach($out as $line)
		{
			$line = trim($line);
			if(empty($line))
				continue;
			list($param, $value) = explode(' = ', $line);
			list($category, $param) = explode(' ', $param);
			$category = explode('/', trim($category, "[] \n"));

			if(count($category) > 1)
				$data[$category[0]][$category[1]][$param] = $value;
			else
				$data[$category[0]][$param] = $value;
		}
		if(isset($data['plugin']['id3v2']['title']))
			$title = $data['plugin']['id3v2']['title'];
		else if(isset($data['plugin']['icymetaint']['title']))
			$title = $data['plugin']['icymetaint']['title'];
		else
			$title = basename($data['server']['url']);

		if(isset($data['plugin']['id3v2']['artist']))
			$title = $data['plugin']['id3v2']['artist'] . ' - ' . $title;

		if(isset($data['plugin']['mad']['duration']))
		{
			$duration = $data['plugin']['mad']['duration'] / 1000 / 60;
			$mins = floor($duration);
			$secs = $duration - $mins;
			$secs = sprintf('%02.0f', round($secs*60));
			$duration = $mins . ':' . $secs;

			$pass = round($data['plugin']['mad']['duration'] / 1000) - (time() - $data['server']['laststarted']);
		}
		else
			$duration = $pass = '';

		if(isset($data['plugin']['mad']))
		{
			$bitrate = round($data['plugin']['mad']['bitrate'] / 1000);
			$sample = $data['plugin']['mad']['samplerate'] / 1000;
		}
		else
			$bitrate = $sample = '';

		if(isset($data['plugin']['curl']['channel']))
		{
			$channel = $data['plugin']['curl']['channel'];
		}
		else
			$channel = '';
		break;
	case 'audacious2':
	default:
		switch(trim(shell_exec('audtool2 playback-status')))
		{
			case 'playing':
				$status = 1;
				break;
			case 'paused':
			case 'stoped':
				$status = 2;
				break;
			default:
				$status = 0;
				break;
		}

		$player = trim(preg_replace('/\[UNS.+\]/iuU', '', shell_exec('audacious2 --version')));

		$title = trim(shell_exec('audtool2 current-song'));
		$bitrate = trim(shell_exec('audtool2 current-song-bitrate-kbps'));
		$sample = trim(shell_exec('audtool2 current-song-frequency-khz'));

		$filename = trim(shell_exec('audtool2 current-song-filename'));
		if(strpos($filename, 'http') === 0)
		{
			$duration = $pass = '';
			$channel = trim(shell_exec('audtool2 current-song-tuple-data album'));
			$data['server']['url'] = $filename;
		}
		else
		{
			$duration = trim(shell_exec('audtool2 current-song-length'));
			$pass =  round((trim(shell_exec('audtool2 current-song-output-length-seconds') / trim(shell_exec('audtool2 current-song-length-seconds')))) * 100);
		}
		break;
}

if($status == 1)
{
	$return = $me ? '+me ' : '';
	$return .= 'отрывается под ' . $title;
	if(!empty($duration))
		$return .= '; Длина песни: ' . $duration;
	if(!empty($pass))
		$return .= '; Уже прослушал: ' . $pass . '%';

	if(!empty($bitrate))
		$return .= ' | <' . $bitrate . 'kbps:' . $sample . 'kHz>';

	if(!empty($channel))
		$return .= ' Радио: ' . $channel . ' | Адрес: ' . $data['server']['url'];

	$return .= " [Powered by $player | GNU/Linux]\n\n";
}
else
{
	switch($status)
	{
		case 2:
			$return = 'Проигрывание остановлено';
			break;
		case 0:
		default:
			$return = 'Аудиоплеер не запущен';
			break;
	}
}

echo $return;
?>
