use Env qw[$HOME];

$config{'separator'}=";";
$config{'eiskaltHostPort'}="localhost:3121";
$config{eiskaltURL} = "http://$config{eiskaltHostPort}/eiskaltdcpp";
$config{'hist_file'}="$HOME/.config/eiskaltdc++/eiskaltdcpp_xcli.hist";
$config{'hist_max'}=500;
$config{'prompt'}="# ";

1;
