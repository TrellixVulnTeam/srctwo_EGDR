<?php
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: google/protobuf/descriptor.proto

namespace Google\Protobuf\Internal;

use Google\Protobuf\Internal\GPBType;
use Google\Protobuf\Internal\GPBWire;
use Google\Protobuf\Internal\RepeatedField;
use Google\Protobuf\Internal\InputStream;
use Google\Protobuf\Internal\GPBUtil;

/**
 * Generated from protobuf message <code>google.protobuf.FieldOptions</code>
 */
class FieldOptions extends \Google\Protobuf\Internal\Message
{
    /**
     * The ctype option instructs the C++ code generator to use a different
     * representation of the field than it normally would.  See the specific
     * options below.  This option is not yet implemented in the open source
     * release -- sorry, we'll try to include it in a future version!
     *
     * Generated from protobuf field <code>optional .google.protobuf.FieldOptions.CType ctype = 1 [default = STRING];</code>
     */
    private $ctype = 0;
    private $has_ctype = false;
    /**
     * The packed option can be enabled for repeated primitive fields to enable
     * a more efficient representation on the wire. Rather than repeatedly
     * writing the tag and type for each element, the entire array is encoded as
     * a single length-delimited blob. In proto3, only explicit setting it to
     * false will avoid using packed encoding.
     *
     * Generated from protobuf field <code>optional bool packed = 2;</code>
     */
    private $packed = false;
    private $has_packed = false;
    /**
     * The jstype option determines the JavaScript type used for values of the
     * field.  The option is permitted only for 64 bit integral and fixed types
     * (int64, uint64, sint64, fixed64, sfixed64).  By default these types are
     * represented as JavaScript strings.  This avoids loss of precision that can
     * happen when a large value is converted to a floating point JavaScript
     * numbers.  Specifying JS_NUMBER for the jstype causes the generated
     * JavaScript code to use the JavaScript "number" type instead of strings.
     * This option is an enum to permit additional types to be added,
     * e.g. goog.math.Integer.
     *
     * Generated from protobuf field <code>optional .google.protobuf.FieldOptions.JSType jstype = 6 [default = JS_NORMAL];</code>
     */
    private $jstype = 0;
    private $has_jstype = false;
    /**
     * Should this field be parsed lazily?  Lazy applies only to message-type
     * fields.  It means that when the outer message is initially parsed, the
     * inner message's contents will not be parsed but instead stored in encoded
     * form.  The inner message will actually be parsed when it is first accessed.
     * This is only a hint.  Implementations are free to choose whether to use
     * eager or lazy parsing regardless of the value of this option.  However,
     * setting this option true suggests that the protocol author believes that
     * using lazy parsing on this field is worth the additional bookkeeping
     * overhead typically needed to implement it.
     * This option does not affect the public interface of any generated code;
     * all method signatures remain the same.  Furthermore, thread-safety of the
     * interface is not affected by this option; const methods remain safe to
     * call from multiple threads concurrently, while non-const methods continue
     * to require exclusive access.
     * Note that implementations may choose not to check required fields within
     * a lazy sub-message.  That is, calling IsInitialized() on the outer message
     * may return true even if the inner message has missing required fields.
     * This is necessary because otherwise the inner message would have to be
     * parsed in order to perform the check, defeating the purpose of lazy
     * parsing.  An implementation which chooses not to check required fields
     * must be consistent about it.  That is, for any particular sub-message, the
     * implementation must either *always* check its required fields, or *never*
     * check its required fields, regardless of whether or not the message has
     * been parsed.
     *
     * Generated from protobuf field <code>optional bool lazy = 5 [default = false];</code>
     */
    private $lazy = false;
    private $has_lazy = false;
    /**
     * Is this field deprecated?
     * Depending on the target platform, this can emit Deprecated annotations
     * for accessors, or it will be completely ignored; in the very least, this
     * is a formalization for deprecating fields.
     *
     * Generated from protobuf field <code>optional bool deprecated = 3 [default = false];</code>
     */
    private $deprecated = false;
    private $has_deprecated = false;
    /**
     * For Google-internal migration only. Do not use.
     *
     * Generated from protobuf field <code>optional bool weak = 10 [default = false];</code>
     */
    private $weak = false;
    private $has_weak = false;
    /**
     * The parser stores options it doesn't recognize here. See above.
     *
     * Generated from protobuf field <code>repeated .google.protobuf.UninterpretedOption uninterpreted_option = 999;</code>
     */
    private $uninterpreted_option;
    private $has_uninterpreted_option = false;

    public function __construct() {
        \GPBMetadata\Google\Protobuf\Internal\Descriptor::initOnce();
        parent::__construct();
    }

    /**
     * The ctype option instructs the C++ code generator to use a different
     * representation of the field than it normally would.  See the specific
     * options below.  This option is not yet implemented in the open source
     * release -- sorry, we'll try to include it in a future version!
     *
     * Generated from protobuf field <code>optional .google.protobuf.FieldOptions.CType ctype = 1 [default = STRING];</code>
     * @return int
     */
    public function getCtype()
    {
        return $this->ctype;
    }

    /**
     * The ctype option instructs the C++ code generator to use a different
     * representation of the field than it normally would.  See the specific
     * options below.  This option is not yet implemented in the open source
     * release -- sorry, we'll try to include it in a future version!
     *
     * Generated from protobuf field <code>optional .google.protobuf.FieldOptions.CType ctype = 1 [default = STRING];</code>
     * @param int $var
     * @return $this
     */
    public function setCtype($var)
    {
        GPBUtil::checkEnum($var, \Google\Protobuf\Internal\FieldOptions_CType::class);
        $this->ctype = $var;
        $this->has_ctype = true;

        return $this;
    }

    public function hasCtype()
    {
        return $this->has_ctype;
    }

    /**
     * The packed option can be enabled for repeated primitive fields to enable
     * a more efficient representation on the wire. Rather than repeatedly
     * writing the tag and type for each element, the entire array is encoded as
     * a single length-delimited blob. In proto3, only explicit setting it to
     * false will avoid using packed encoding.
     *
     * Generated from protobuf field <code>optional bool packed = 2;</code>
     * @return bool
     */
    public function getPacked()
    {
        return $this->packed;
    }

    /**
     * The packed option can be enabled for repeated primitive fields to enable
     * a more efficient representation on the wire. Rather than repeatedly
     * writing the tag and type for each element, the entire array is encoded as
     * a single length-delimited blob. In proto3, only explicit setting it to
     * false will avoid using packed encoding.
     *
     * Generated from protobuf field <code>optional bool packed = 2;</code>
     * @param bool $var
     * @return $this
     */
    public function setPacked($var)
    {
        GPBUtil::checkBool($var);
        $this->packed = $var;
        $this->has_packed = true;

        return $this;
    }

    public function hasPacked()
    {
        return $this->has_packed;
    }

    /**
     * The jstype option determines the JavaScript type used for values of the
     * field.  The option is permitted only for 64 bit integral and fixed types
     * (int64, uint64, sint64, fixed64, sfixed64).  By default these types are
     * represented as JavaScript strings.  This avoids loss of precision that can
     * happen when a large value is converted to a floating point JavaScript
     * numbers.  Specifying JS_NUMBER for the jstype causes the generated
     * JavaScript code to use the JavaScript "number" type instead of strings.
     * This option is an enum to permit additional types to be added,
     * e.g. goog.math.Integer.
     *
     * Generated from protobuf field <code>optional .google.protobuf.FieldOptions.JSType jstype = 6 [default = JS_NORMAL];</code>
     * @return int
     */
    public function getJstype()
    {
        return $this->jstype;
    }

    /**
     * The jstype option determines the JavaScript type used for values of the
     * field.  The option is permitted only for 64 bit integral and fixed types
     * (int64, uint64, sint64, fixed64, sfixed64).  By default these types are
     * represented as JavaScript strings.  This avoids loss of precision that can
     * happen when a large value is converted to a floating point JavaScript
     * numbers.  Specifying JS_NUMBER for the jstype causes the generated
     * JavaScript code to use the JavaScript "number" type instead of strings.
     * This option is an enum to permit additional types to be added,
     * e.g. goog.math.Integer.
     *
     * Generated from protobuf field <code>optional .google.protobuf.FieldOptions.JSType jstype = 6 [default = JS_NORMAL];</code>
     * @param int $var
     * @return $this
     */
    public function setJstype($var)
    {
        GPBUtil::checkEnum($var, \Google\Protobuf\Internal\FieldOptions_JSType::class);
        $this->jstype = $var;
        $this->has_jstype = true;

        return $this;
    }

    public function hasJstype()
    {
        return $this->has_jstype;
    }

    /**
     * Should this field be parsed lazily?  Lazy applies only to message-type
     * fields.  It means that when the outer message is initially parsed, the
     * inner message's contents will not be parsed but instead stored in encoded
     * form.  The inner message will actually be parsed when it is first accessed.
     * This is only a hint.  Implementations are free to choose whether to use
     * eager or lazy parsing regardless of the value of this option.  However,
     * setting this option true suggests that the protocol author believes that
     * using lazy parsing on this field is worth the additional bookkeeping
     * overhead typically needed to implement it.
     * This option does not affect the public interface of any generated code;
     * all method signatures remain the same.  Furthermore, thread-safety of the
     * interface is not affected by this option; const methods remain safe to
     * call from multiple threads concurrently, while non-const methods continue
     * to require exclusive access.
     * Note that implementations may choose not to check required fields within
     * a lazy sub-message.  That is, calling IsInitialized() on the outer message
     * may return true even if the inner message has missing required fields.
     * This is necessary because otherwise the inner message would have to be
     * parsed in order to perform the check, defeating the purpose of lazy
     * parsing.  An implementation which chooses not to check required fields
     * must be consistent about it.  That is, for any particular sub-message, the
     * implementation must either *always* check its required fields, or *never*
     * check its required fields, regardless of whether or not the message has
     * been parsed.
     *
     * Generated from protobuf field <code>optional bool lazy = 5 [default = false];</code>
     * @return bool
     */
    public function getLazy()
    {
        return $this->lazy;
    }

    /**
     * Should this field be parsed lazily?  Lazy applies only to message-type
     * fields.  It means that when the outer message is initially parsed, the
     * inner message's contents will not be parsed but instead stored in encoded
     * form.  The inner message will actually be parsed when it is first accessed.
     * This is only a hint.  Implementations are free to choose whether to use
     * eager or lazy parsing regardless of the value of this option.  However,
     * setting this option true suggests that the protocol author believes that
     * using lazy parsing on this field is worth the additional bookkeeping
     * overhead typically needed to implement it.
     * This option does not affect the public interface of any generated code;
     * all method signatures remain the same.  Furthermore, thread-safety of the
     * interface is not affected by this option; const methods remain safe to
     * call from multiple threads concurrently, while non-const methods continue
     * to require exclusive access.
     * Note that implementations may choose not to check required fields within
     * a lazy sub-message.  That is, calling IsInitialized() on the outer message
     * may return true even if the inner message has missing required fields.
     * This is necessary because otherwise the inner message would have to be
     * parsed in order to perform the check, defeating the purpose of lazy
     * parsing.  An implementation which chooses not to check required fields
     * must be consistent about it.  That is, for any particular sub-message, the
     * implementation must either *always* check its required fields, or *never*
     * check its required fields, regardless of whether or not the message has
     * been parsed.
     *
     * Generated from protobuf field <code>optional bool lazy = 5 [default = false];</code>
     * @param bool $var
     * @return $this
     */
    public function setLazy($var)
    {
        GPBUtil::checkBool($var);
        $this->lazy = $var;
        $this->has_lazy = true;

        return $this;
    }

    public function hasLazy()
    {
        return $this->has_lazy;
    }

    /**
     * Is this field deprecated?
     * Depending on the target platform, this can emit Deprecated annotations
     * for accessors, or it will be completely ignored; in the very least, this
     * is a formalization for deprecating fields.
     *
     * Generated from protobuf field <code>optional bool deprecated = 3 [default = false];</code>
     * @return bool
     */
    public function getDeprecated()
    {
        return $this->deprecated;
    }

    /**
     * Is this field deprecated?
     * Depending on the target platform, this can emit Deprecated annotations
     * for accessors, or it will be completely ignored; in the very least, this
     * is a formalization for deprecating fields.
     *
     * Generated from protobuf field <code>optional bool deprecated = 3 [default = false];</code>
     * @param bool $var
     * @return $this
     */
    public function setDeprecated($var)
    {
        GPBUtil::checkBool($var);
        $this->deprecated = $var;
        $this->has_deprecated = true;

        return $this;
    }

    public function hasDeprecated()
    {
        return $this->has_deprecated;
    }

    /**
     * For Google-internal migration only. Do not use.
     *
     * Generated from protobuf field <code>optional bool weak = 10 [default = false];</code>
     * @return bool
     */
    public function getWeak()
    {
        return $this->weak;
    }

    /**
     * For Google-internal migration only. Do not use.
     *
     * Generated from protobuf field <code>optional bool weak = 10 [default = false];</code>
     * @param bool $var
     * @return $this
     */
    public function setWeak($var)
    {
        GPBUtil::checkBool($var);
        $this->weak = $var;
        $this->has_weak = true;

        return $this;
    }

    public function hasWeak()
    {
        return $this->has_weak;
    }

    /**
     * The parser stores options it doesn't recognize here. See above.
     *
     * Generated from protobuf field <code>repeated .google.protobuf.UninterpretedOption uninterpreted_option = 999;</code>
     * @return \Google\Protobuf\Internal\RepeatedField
     */
    public function getUninterpretedOption()
    {
        return $this->uninterpreted_option;
    }

    /**
     * The parser stores options it doesn't recognize here. See above.
     *
     * Generated from protobuf field <code>repeated .google.protobuf.UninterpretedOption uninterpreted_option = 999;</code>
     * @param \Google\Protobuf\Internal\UninterpretedOption[]|\Google\Protobuf\Internal\RepeatedField $var
     * @return $this
     */
    public function setUninterpretedOption($var)
    {
        $arr = GPBUtil::checkRepeatedField($var, \Google\Protobuf\Internal\GPBType::MESSAGE, \Google\Protobuf\Internal\UninterpretedOption::class);
        $this->uninterpreted_option = $arr;
        $this->has_uninterpreted_option = true;

        return $this;
    }

    public function hasUninterpretedOption()
    {
        return $this->has_uninterpreted_option;
    }

}

