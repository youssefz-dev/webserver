#!/usr/bin/php-cgi
<?php

$method      = $_SERVER['REQUEST_METHOD'] ?? 'GET';
$script_name = $_SERVER['SCRIPT_NAME']    ?? '/cgi/post_test.php';
$remote_addr = $_SERVER['REMOTE_ADDR']    ?? '—';
$server_name = $_SERVER['SERVER_NAME']    ?? '—';
$server_port = $_SERVER['SERVER_PORT']    ?? '—';
$user_agent  = $_SERVER['HTTP_USER_AGENT'] ?? '—';
$content_type   = $_SERVER['CONTENT_TYPE']   ?? '—';
$content_length = $_SERVER['CONTENT_LENGTH'] ?? '0';

// ── helpers ───────────────────────────────────────────────────────────────────
function esc($s) {
    return htmlspecialchars((string)$s, ENT_QUOTES, 'UTF-8');
}
function result_row($key, $val) {
    return '<div class="result-row">'
         . '<div class="result-key">' . esc($key) . '</div>'
         . '<div class="result-val">' . esc($val)  . '</div>'
         . '</div>';
}
function env_line($key, $val) {
    return '<div class="env-line">'
         . '<span class="k">' . esc($key) . '</span>&nbsp;&nbsp;'
         . '<span class="v">' . esc($val) . '</span>'
         . '</div>';
}

// ── build body ────────────────────────────────────────────────────────────────
if ($method === 'POST') {

    // Read raw POST body from stdin
    $stdin     = fopen('php://stdin', 'r');
    $post_body = stream_get_contents($stdin);
    fclose($stdin);

    parse_str($post_body, $params);

    $name = $params['name'] ?? 'Guest';
    $age  = $params['age']  ?? 'unknown';

    $accent       = '#00e5a0';
    $glow         = 'rgba(0,229,160,.05)';
    $pulse_color  = 'rgba(0,229,160,.4)';
    $badge_border = 'rgba(0,229,160,.25)';
    $status_dot   = 's-ok';
    $status_label = 'POST OK';
    $title        = 'POST Received';

    $fields_html  = '<span class="section-label">Parsed Fields</span>';
    $fields_html .= '<div class="result-block">';
    if (!empty($params)) {
        foreach ($params as $k => $v) {
            $fields_html .= result_row($k, $v);
        }
    } else {
        $fields_html .= '<div class="result-row"><div class="result-val" style="opacity:.4">No fields received</div></div>';
    }
    $fields_html .= '</div>';

    $env_html  = '<span class="section-label">Request Info</span>';
    $env_html .= '<div class="env-block">';
    $env_html .= env_line('METHOD',         $method);
    $env_html .= env_line('CONTENT_TYPE',   $content_type);
    $env_html .= env_line('CONTENT_LENGTH', $content_length . ' bytes');
    $env_html .= env_line('REMOTE_ADDR',    $remote_addr);
    $env_html .= env_line('SERVER',         $server_name . ':' . $server_port);
    $env_html .= env_line('PHP_VERSION',    phpversion());
    $env_html .= '</div>';

    $body_html = $fields_html . $env_html;

} else {

    $accent       = '#f5a623';
    $glow         = 'rgba(245,166,35,.05)';
    $pulse_color  = 'rgba(245,166,35,.4)';
    $badge_border = 'rgba(245,166,35,.25)';
    $status_dot   = 's-warn';
    $status_label = 'Awaiting POST';
    $title        = 'Awaiting POST';

    $body_html  = '<span class="section-label">Instructions</span>';
    $body_html .= '<div class="result-block">';
    $body_html .= result_row('expected', 'POST');
    $body_html .= result_row('fields',   'name, age');
    $body_html .= result_row('encoding', 'application/x-www-form-urlencoded');
    $body_html .= result_row('received', $method);
    $body_html .= '</div>';

    $body_html .= '<span class="section-label" style="margin-top:1rem;display:block">Environment</span>';
    $body_html .= '<div class="env-block">';
    $body_html .= env_line('METHOD',      $method);
    $body_html .= env_line('REMOTE_ADDR', $remote_addr);
    $body_html .= env_line('SERVER',      $server_name . ':' . $server_port);
    $body_html .= env_line('PHP_VERSION', phpversion());
    $body_html .= '</div>';
}

// ── output ────────────────────────────────────────────────────────────────────
header('Content-Type: text/html; charset=UTF-8');
?>
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8"/>
<meta name="viewport" content="width=device-width,initial-scale=1"/>
<title>CGI — <?= esc(basename($script_name)) ?></title>
<link rel="preconnect" href="https://fonts.googleapis.com"/>
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin/>
<link href="https://fonts.googleapis.com/css2?family=Bebas+Neue&family=Syne:wght@400;700;800&family=DM+Mono:wght@300;400&display=swap" rel="stylesheet"/>
<style>
*,*::before,*::after{box-sizing:border-box;margin:0;padding:0}
:root{
  --bg:#080c10;--border:rgba(255,255,255,.05);
  --text:#e8edf2;--muted:rgba(232,237,242,.35);
  --accent:<?= $accent ?>;--surface:rgba(13,19,24,.9);
}
html,body{height:100%;overflow:hidden;background:var(--bg);color:var(--text);font-family:'Syne',sans-serif}
body::before{
  content:'';position:fixed;inset:0;z-index:0;
  background-image:
    linear-gradient(var(--border) 1px,transparent 1px),
    linear-gradient(90deg,var(--border) 1px,transparent 1px);
  background-size:56px 56px;
  mask-image:radial-gradient(ellipse 90% 90% at 50% 50%,black 20%,transparent 100%);
  pointer-events:none;
}
body::after{
  content:'';position:fixed;width:600px;height:600px;
  top:50%;left:50%;transform:translate(-50%,-50%);
  background:radial-gradient(circle,<?= $glow ?> 0%,transparent 70%);
  pointer-events:none;z-index:0;
}
.page{position:relative;z-index:1;min-height:100vh;display:flex;flex-direction:column}
.bar{
  display:flex;align-items:center;justify-content:space-between;
  padding:1rem 2rem;border-bottom:1px solid var(--border);flex-shrink:0;
}
.bar-left{display:flex;align-items:center;gap:.8rem}
.dot{width:8px;height:8px;border-radius:50%;background:var(--accent);animation:pulse 2s ease-in-out infinite}
@keyframes pulse{0%,100%{opacity:1;box-shadow:0 0 0 0 <?= $pulse_color ?>}50%{opacity:.7;box-shadow:0 0 0 5px rgba(0,0,0,0)}}
.bar-label{font-family:'DM Mono',monospace;font-size:.72rem;color:var(--accent);letter-spacing:.14em}
.bar-right{font-family:'DM Mono',monospace;font-size:.68rem;color:var(--muted);letter-spacing:.1em}
.back{font-family:'DM Mono',monospace;font-size:.68rem;color:var(--muted);letter-spacing:.1em;text-decoration:none;transition:color .2s}
.back:hover{color:var(--text)}
main{flex:1;display:flex;align-items:center;justify-content:center;padding:2rem;position:relative}
.card{
  width:100%;max-width:540px;background:var(--surface);
  border:1px solid var(--border);border-top:2px solid var(--accent);
  border-radius:4px;backdrop-filter:blur(16px);animation:fadeup .5s ease both;
}
@keyframes fadeup{from{opacity:0;transform:translateY(16px)}to{opacity:1;transform:translateY(0)}}
.card-header{
  padding:1.6rem 2rem 1.2rem;border-bottom:1px solid var(--border);
  display:flex;align-items:flex-start;justify-content:space-between;
}
.tag{
  display:inline-flex;align-items:center;gap:.4rem;
  font-family:'DM Mono',monospace;font-size:.64rem;
  letter-spacing:.18em;text-transform:uppercase;color:var(--accent);margin-bottom:.5rem;
}
.tag::before{content:'';width:4px;height:4px;border-radius:50%;background:var(--accent)}
.card-title{font-family:'Bebas Neue',sans-serif;font-size:2rem;letter-spacing:.06em;color:var(--text)}
.method-badge{
  font-family:'DM Mono',monospace;font-size:.66rem;letter-spacing:.14em;
  color:var(--accent);border:1px solid <?= $badge_border ?>;
  padding:.25rem .6rem;border-radius:2px;margin-top:.2rem;
}
.card-body{padding:1.8rem 2rem}
.section-label{
  display:block;font-family:'DM Mono',monospace;font-size:.64rem;
  letter-spacing:.16em;text-transform:uppercase;color:var(--muted);margin-bottom:.6rem;
}
.result-block{border:1px solid var(--border);border-radius:2px;overflow:hidden;margin-bottom:1.2rem}
.result-row{display:flex;align-items:stretch;border-bottom:1px solid var(--border)}
.result-row:last-child{border-bottom:none}
.result-key{
  font-family:'DM Mono',monospace;font-size:.72rem;color:var(--accent);letter-spacing:.06em;
  padding:.6rem .9rem;background:rgba(255,255,255,.02);
  border-right:1px solid var(--border);min-width:130px;flex-shrink:0;
}
.result-val{
  font-family:'DM Mono',monospace;font-size:.72rem;color:var(--text);letter-spacing:.04em;
  padding:.6rem .9rem;flex:1;word-break:break-all;
}
.env-block{
  background:rgba(255,255,255,.015);border:1px solid var(--border);
  border-left:2px solid rgba(255,255,255,.08);border-radius:0 2px 2px 0;
  padding:.9rem 1rem;margin-bottom:1.2rem;
}
.env-line{font-family:'DM Mono',monospace;font-size:.7rem;color:var(--muted);letter-spacing:.04em;line-height:2}
.env-line .k{color:rgba(255,255,255,.25)}
.env-line .v{color:var(--text)}
.divider{height:1px;background:var(--border);margin:1.2rem 0}
.actions{display:flex;gap:.6rem}
.btn{
  flex:1;padding:.8rem;border:none;border-radius:2px;
  font-family:'DM Mono',monospace;font-size:.74rem;
  letter-spacing:.14em;text-transform:uppercase;
  cursor:pointer;transition:opacity .2s,transform .2s;
  display:flex;align-items:center;justify-content:center;gap:.5rem;text-decoration:none;
}
.btn:hover{opacity:.85;transform:translateY(-1px)}
.btn-primary{background:var(--accent);color:#080c10;font-weight:700}
.btn-ghost{background:transparent;color:var(--muted);border:1px solid var(--border)}
.btn-ghost:hover{border-color:rgba(255,255,255,.15);color:var(--text)}
.bottombar{
  display:flex;align-items:center;justify-content:space-between;
  padding:.8rem 2rem;border-top:1px solid var(--border);
  font-family:'DM Mono',monospace;font-size:.66rem;color:var(--muted);letter-spacing:.1em;
  flex-shrink:0;
}
.status-row{display:flex;align-items:center;gap:1.5rem}
.status-item{display:flex;align-items:center;gap:.4rem}
.sdot{width:6px;height:6px;border-radius:50%}
.s-ok{background:#00e5a0}.s-warn{background:#f5a623}
</style>
</head>
<body>
<div class="page">

  <div class="bar">
    <div class="bar-left">
      <div class="dot"></div>
      <span class="bar-label"><?= esc($method) ?> <?= esc($script_name) ?></span>
    </div>
    <div style="display:flex;align-items:center;gap:2rem">
      <a href="../cgi.html" class="back">← CGI Tester</a>
      <span class="bar-right" id="ts"></span>
    </div>
  </div>

  <main>
    <div class="card">
      <div class="card-header">
        <div>
          <div class="tag">PHP · CGI Response</div>
          <div class="card-title"><?= esc($title) ?></div>
        </div>
        <div class="method-badge"><?= esc($method) ?></div>
      </div>
      <div class="card-body">
        <?= $body_html ?>
        <div class="divider"></div>
        <div class="actions">
          <a href="../cgi.html" class="btn btn-primary">← Back to CGI Tester</a>
          <a href="../index.html" class="btn btn-ghost">↩ Dashboard</a>
        </div>
      </div>
    </div>
  </main>

  <div class="bottombar">
    <div class="status-row">
      <div class="status-item">
        <div class="sdot <?= $status_dot ?>"></div>
        <?= esc($status_label) ?>
      </div>
    </div>
    <span><?= esc($script_name) ?></span>
  </div>

</div>
<script>
  const pad = n => String(n).padStart(2,'0');
  const tick = () => {
    const d = new Date();
    document.getElementById('ts').textContent =
      `${d.getFullYear()}-${pad(d.getMonth()+1)}-${pad(d.getDate())}  ${pad(d.getHours())}:${pad(d.getMinutes())}:${pad(d.getSeconds())}`;
  };
  tick(); setInterval(tick,1000);
</script>
</body>
</html>