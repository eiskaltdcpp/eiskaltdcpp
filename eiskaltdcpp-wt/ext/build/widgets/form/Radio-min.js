/*
 * Ext JS Library 2.0.2
 * Copyright(c) 2006-2008, Ext JS, LLC.
 * licensing@extjs.com
 * 
 * http://extjs.com/license
 */

Ext.form.Radio=Ext.extend(Ext.form.Checkbox,{inputType:"radio",markInvalid:Ext.emptyFn,clearInvalid:Ext.emptyFn,getGroupValue:function(){var A=this.el.up("form")||Ext.getBody();var B=A.child("input[name="+this.el.dom.name+"]:checked",true);return B?B.value:null},onClick:function(){if(this.el.dom.checked!=this.checked){var B=this.el.up("form")||Ext.getBody();var A=B.select("input[name="+this.el.dom.name+"]");A.each(function(C){if(C.dom.id==this.id){this.setValue(true)}else{Ext.getCmp(C.dom.id).setValue(false)}},this)}},setValue:function(A){if(typeof A=="boolean"){Ext.form.Radio.superclass.setValue.call(this,A)}else{var B=this.el.up("form").child("input[name="+this.el.dom.name+"][value="+A+"]",true);if(B){B.checked=true}}}});Ext.reg("radio",Ext.form.Radio);