/*
 * Ext JS Library 2.0.2
 * Copyright(c) 2006-2008, Ext JS, LLC.
 * licensing@extjs.com
 * 
 * http://extjs.com/license
 */

Ext.QuickTips=function(){var B,A=[];return{init:function(){if(!B){B=new Ext.QuickTip({elements:"header,body"})}},enable:function(){if(B){A.pop();if(A.length<1){B.enable()}}},disable:function(){if(B){B.disable()}A.push(1)},isEnabled:function(){return B&&!B.disabled},getQuickTip:function(){return B},register:function(){B.register.apply(B,arguments)},unregister:function(){B.unregister.apply(B,arguments)},tips:function(){B.register.apply(B,arguments)}}}();