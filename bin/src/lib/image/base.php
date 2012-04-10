<?
if(!defined('IN_SITE'))
	exit('Access Denied');

/**
 * Enter description here ...
 * @author Administrator
 *
 */
class LibImageBase{
	var $info;

	/**
	 * Enter description here ...
	 * @param unknown_type $img
	 * @return multitype:unknown string number Ambigous <> |boolean
	 */
	function getInfo($img){
		$imageInfo=getimagesize($img);
		if( $imageInfo!==false){
			$imageType=strtolower(substr(image_type_to_extension($imageInfo[2]),1));
			$imageSize=filesize($img);
			return array(
				"width"=>$imageInfo[0],
				"height"=>$imageInfo[1],
				"type"=>$imageType,
				"size"=>$imageSize,
				"mime"=>$imageInfo['mime']
			);
		}else{
			return false;
		}
	}

	/**
	 * Enter description here ...
	 * @param unknown_type $im
	 * @param unknown_type $type
	 * @param unknown_type $filename
	 */
	function output($im,$type='png',$filename=''){
		header("Content-type: image/".$type);
		$ImageFun='image'.$type;
		if(empty($filename)){
			$ImageFun($im);
		}else{
			$ImageFun($im,$filename);
		}
		imagedestroy($im);
	}
}
