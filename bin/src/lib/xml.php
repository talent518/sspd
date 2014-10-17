<?php
if (  ! defined('IN_SERVER') )
	exit('Access Denied');
	
	/*
 * XML和php对象的相互转换 @version 1.3 @author ABao <talent518@live.cn> 2个函数2个类，如下： @function xml_to_object @function object_to_xml @function array_to_xml @class XML_Element
 */

/*
 * XML文本转为php对象
 *
 * @param string $xml
 * @return XML_Element对象
 */
function xml_to_object ( $xml, $isMultiRoot = false, &$error = false ) {
	$data = $xml;
	if ( $isMultiRoot ) {
		$tags = array();
		do {
			$parser = xml_parser_create();
			xml_parser_set_option($parser, XML_OPTION_CASE_FOLDING, 0);
			xml_parser_set_option($parser, XML_OPTION_SKIP_WHITE, 1);
			xml_parser_set_option($parser, XML_OPTION_TARGET_ENCODING, 'utf-8'); // 设置该 XML 解析器所使用的目标编码
			$_tags = array();
			$result = xml_parse_into_struct($parser, $data, $_tags);
			if ( $result ) {
				foreach ( $_tags as $v ) {
					$tags[] = $v;
				}
				$_tags = null;
				xml_parser_free($parser);
				break;
			}
			if ( xml_get_error_code($parser) === 5 && ( $bindex = xml_get_current_byte_index($parser) ) && $bindex != strlen($data) ) {
				$parser = xml_parser_create();
				xml_parser_set_option($parser, XML_OPTION_CASE_FOLDING, 0);
				xml_parser_set_option($parser, XML_OPTION_SKIP_WHITE, 1);
				xml_parser_set_option($parser, XML_OPTION_TARGET_ENCODING, 'utf-8'); // 设置该 XML 解析器所使用的目标编码
				if (  !  ! ( $result = xml_parse_into_struct($parser, substr($data, 0, $bindex), $_tags) ) ) {
					foreach ( $_tags as $v ) {
						$tags[] = $v;
					}
					$_tags = null;
					$data = trim(substr($data, $bindex));
				} else {
					$data = trim(substr($data, 0, $bindex));
				}
			}
			xml_parser_free($parser);
		} while ( $result && $data );
	} else {
		$parser = xml_parser_create();
		xml_parser_set_option($parser, XML_OPTION_CASE_FOLDING, 0);
		xml_parser_set_option($parser, XML_OPTION_SKIP_WHITE, 1);
		xml_parser_set_option($parser, XML_OPTION_TARGET_ENCODING, 'utf-8'); // 设置该 XML 解析器所使用的目标编码
		$result = xml_parse_into_struct($parser, $data, $tags);
	}
	if (  ! $result ) {
		$code = xml_get_error_code($parser);
		$error = array(
			'code' => $code, 
			'data' => $data, 
			'message' => xml_error_string($code), 
			'line' => xml_get_current_line_number($parser), 
			'column' => xml_get_current_column_number($parser), 
			'index' => xml_get_current_byte_index($parser)
		);
	} else {
		$error = false;
	}
	
	xml_parser_free($parser);
	$parser = null;
	
	if (  ! $tags )
		return false;
	
	$stack = array();
	$element = ( $isMultiRoot ? array() : null );
	
	foreach ( $tags as $tag ) {
		if ( $tag['type'] == 'complete' || $tag['type'] == 'open' ) {
			$e = new XML_Element($tag['tag']);
			
			if ( isset($tag['attributes']) ) {
				foreach ( $tag['attributes'] as $k => $v )
					$e->$k = $v;
			}
			
			if ( isset($tag['value']) )
				$e->setText($tag['value']);
			
			if ( count($stack) )
				$elem = $stack[count($stack) - 1];
			else
				$elem = &$element;
			
			if ( is_array($elem) ) {
				array_push($elem, $e);
			} elseif ( $elem !== null ) {
				if ( $elem->getTag() == $tag['tag'] . 's' ) {
					$elem->addChild($e);
				} else {
					$elem->$tag['tag'] = $e;
				}
			} elseif ( is_array($element) ) {
				array_push($elem, $e);
			} else {
				$element = $e;
			}
			
			if ( $tag['type'] == 'open' )
				$stack[count($stack)] = $e;
			
			unset($elem, $e);
		}
		if ( $tag['type'] == 'close' ) {
			array_pop($stack);
		}
	}
	$tags = $stack = null;
	return $element; // the single top-level element
}

/*
 * php对象转为XML文本 @param string $object XML_Element对象 @param boolean $head 是否包括xml头信息 @return XML文本
 */
function object_to_xml ( &$object, $head = false ) {
	$return = ( $head === true ? '<?xml version="1.0" encoding="utf-8"?>' : '' );
	if ( $object instanceof XML_Element ) {
		$p = $x = '';
		foreach ( $object as $k => $v ) {
			if ( is_object($v) )
				$x .= object_to_xml($v, false);
			elseif ( is_array($v) )
				$x .= sprintf('<%s>%s</%s>', $k, object_to_xml($v, false), $k);
			else
				$p .= sprintf(' %s="%s"', $k, object_to_xml($v, false));
		}
		if ( $object->getText() && preg_match('/\s|\&|\<|\>/', $object->getText()) ) {
			$c = str_replace(array(
				'<![CDATA[', 
				']]>'
			), array(
				'&lt;![CDATA[', 
				']]&gt;'
			), $object->getText());
			$c = sprintf('<![CDATA[%s]]>', $c);
		} else
			$c = $object->getText();
		$return .= sprintf($x . $c == '' ? '<%s%s/>' : '<%s%s>%s</%s>', $object->getTag(), $p, $x . $c, $object->getTag());
	} elseif ( is_array($object) ) {
		foreach ( $object as $k => $v ) {
			if ( is_int($k) )
				$return .= object_to_xml($v, false);
			else
				$return .= sprintf('<%s>%s</%s>', $k, object_to_xml($v, false), $k);
		}
	} elseif ( is_bool($object) ) {
		$return .= ( $object ? 'true' : 'false' );
	} else {
		$return .= htmlspecialchars($object);
	}
	return $return;
}

function array_to_xml ( $array, $tagname, $textkey = '' ) {
	$xml = new XML_Element($tagname);
	foreach ( $array as $key => $value ) {
		if (  ! empty($textkey) && $textkey == $key ) {
			$xml->setText($value);
		}
		if ( is_string($key) )
			$xml->$key = $value;
	}
	return $xml;
}

class XML_Element extends stdClass {

	private $__text, $__tag, $__length = 0, $hasAttr = false;

	function XML_Element ( $tag ) {
		$this->__tag = $tag;
	}

	function setText ( $text ) {
		$this->__text = ( string ) $text;
	}

	function getText () {
		return $this->__text;
	}

	function setTag ( $tag ) {
		$this->__tag = $tag;
	}

	function getTag () {
		return $this->__tag;
	}

	function getLength () {
		return $this->__length;
	}

	function addChild ( $child ) {
		$this->{$this->__length ++  } = $child;
		$this->hasAttr = true;
	}

	function removeChild ( $child ) {
		foreach ( $this as $k => $v ) {
			if ( $child == $v ) {
				$this->$k = null;
				unset($this->$k);
				return $k;
			}
		}
		return false;
	}

	function removeChildAt ( $key ) {
		if ( property_exists($this, $key) ) {
			$ret = $this->$key;
			$this->$key = null;
			unset($this->$key);
			return $ret;
		} else {
			return false;
		}
	}

	function addAttr ( $k, $v ) {
		$this->$k = $v;
	}

	function removeAttr ( $k ) {
		return $this->removeChildAt($k);
	}

	function getAttr ( $k ) {
		if ( $this->$k instanceof XML_Element ) {
			$this->$k->getText();
		} else {
			return $this->$k;
		}
	}

	function __set ( $key, $value ) {
		$this->$key = $value;
		$this->hasAttr = true;
	}

	function __toString () {
		$return = ( $this->hasAttr ? object_to_xml($this) : $this->__text );
		return ( string ) $return;
	}

	function __destruct () {
		foreach ( $this as $k => $v ) {
			// echo '__destruct:',$k,'=>',$v,PHP_EOL;
			$this->$k = $v = null;
		}
	}

}
