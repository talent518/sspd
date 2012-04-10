<?
if(!defined('IN_SITE'))
	exit('Access Denied');

import('lib.image.base');

class LibImageUpca extends LibImageBase{
	/**
	 +----------------------------------------------------------
	 * 生成UPC-A条形码
	 +----------------------------------------------------------
	 * @static
	 +----------------------------------------------------------
	 * @param string $type 图像格式
	 * @param string $type 图像格式
	 * @param string $lw  单元宽度
	 * @param string $hi   条码高度
	 +----------------------------------------------------------
	 * @return string
	 +----------------------------------------------------------
	 */
	function UPCA($code,$type='png',$lw=2,$hi=100){
		static $Lencode=array('0001101','0011001','0010011','0111101','0100011','0110001','0101111','0111011','0110111','0001011');
		static $Rencode=array('1110010','1100110','1101100','1000010','1011100','1001110','1010000','1000100','1001000','1110100');
		$ends='101';
		$center='01010';
		/* UPC-A Must be 11	digits,	we compute the checksum. */
		if(strlen($code) !=11 ){ die("UPC-A	Must be	11 digits."); }
		/* Compute the EAN-13 Checksum digit */
		$ncode='0'.$code;
		$even=0; $odd=0;
		for	($x=0;$x<12;$x++){
		  if ($x % 2){ $odd	+=$ncode[$x]; }	else{ $even	+=$ncode[$x]; }
		}
		$code.=(10 - (($odd	* 3	+ $even) % 10))	% 10;
		/* Create the bar encoding using a binary string */
		$bars=$ends;
		$bars.=$Lencode[$code[0]];
		for($x=1;$x<6;$x++){
		  $bars.=$Lencode[$code[$x]];
		}
		$bars.=$center;
		for($x=6;$x<12;$x++){
		  $bars.=$Rencode[$code[$x]];
		}
		$bars.=$ends;
		/* Generate	the	Barcode	Image */
		if($type!='gif'	&& function_exists('imagecreatetruecolor')){
			$im=imagecreatetruecolor($lw*95+30,$hi+30);
		}else{
			$im=imagecreate($lw*95+30,$hi+30);
		}
		$fg=ImageColorAllocate($im,	0, 0, 0);
		$bg=ImageColorAllocate($im,	255, 255, 255);
		ImageFilledRectangle($im, 0, 0,	$lw*95+30, $hi+30, $bg);
		$shift=10;
		for	($x=0;$x<strlen($bars);$x++){
		  if (($x<10) || ($x>=45 &&	$x<50) || ($x >=85)){ $sh=10; }	else{ $sh=0; }
		  if ($bars[$x]=='1'){ $color=$fg; } else{ $color=$bg; }
		  ImageFilledRectangle($im,	($x*$lw)+15,5,($x+1)*$lw+14,$hi+5+$sh,$color);
		}
		/* Add the Human Readable Label	*/
		ImageString($im,4,5,$hi-5,$code[0],$fg);
		for	($x=0;$x<5;$x++){
		  ImageString($im,5,$lw*(13+$x*6)+15,$hi+5,$code[$x+1],$fg);
		  ImageString($im,5,$lw*(53+$x*6)+15,$hi+5,$code[$x+6],$fg);
		}
		ImageString($im,4,$lw*95+17,$hi-5,$code[11],$fg);
		/* Output the Header and Content. */
		$this->output($im,$type);
	}
}
