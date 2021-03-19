## bimi

BIMI usage

```
import { bimi } from "bimi/bimi.hsl";
import { dmarc } from "dmarc/dmarc.hsl";

$dmarc = dmarc($mail, $senderip, $senderhelo, $senderdomain);
$bimi = bimi($mail, $dmarc);
if ($bimi and $bimi["l"] != "")
{
  $svg = http($bimi["l"], ["timeout" => 5, "max_file_size" => 512000, "tls_default_ca" => true]);
  if ($svg)
  {
	  $base64 = str_strip(array_join(pcre_match_all("/(.{0,64})/", base64_encode($svg))[1], "\r\n "));
		$mail->addHeader("BIMI-Indicator", $base64, ["encode" => false]);
	}
}
```
