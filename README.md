## Brand Indicators for Message Identification

Usage

```
import $calist from "bimi/ca.crt";
import { bimi, bimi_vmc } from "bimi/bimi.hsl";
import { dmarc } from "dmarc/dmarc.hsl";

$dmarc = dmarc($mail, $senderip, $senderhelo, $senderdomain);
$bimi = bimi($mail, $dmarc);

// Verified
if ($bimi["record"]["a"])
{
	$bimi_vmc = bimi_vmc($bimi, $calist);
	if ($bimi_vmc["indicator"])
	{
		$mail->addHeader("BIMI-Indicator",
			str_strip(array_join(pcre_match_all(#/(^(.{0,49})|(.{0,64}))/, $bimi_vmc["indicator"])[1], "\r\n ")),
			["encode" => false]);
	}
	else
		echo "BIMI VMC error: $bimi_vmc; $bimi";
}
else
{
	if ($bimi["error"] and $bimi["class"] != "dmarc" and $bimi["class"] != "dns")
		echo "BIMI error: $bimi";
}

/*
// Unverified
if ($bimi["record"]["l"])
{
	$svg = http($bimi["record"]["l"], ["timeout" => 5, "max_file_size" => 512000, "tls_default_ca" => true]);
	if ($svg)
	{
		$mail->addHeader("BIMI-Indicator",
			str_strip(array_join(pcre_match_all(#/(^(.{0,49})|(.{0,64}))/, base64_encode($svg))[1], "\r\n ")),
			["encode" => false]);
	}
}
*/
```
