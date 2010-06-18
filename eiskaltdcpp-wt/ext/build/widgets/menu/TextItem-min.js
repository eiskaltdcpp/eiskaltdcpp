/*
 * Ext JS Library 2.0.2
 * Copyright(c) 2006-2008, Ext JS, LLC.
 * licensing@extjs.com
 * 
 * http://extjs.com/license
 */

Ext.menu.TextItem=function(A){this.text=A;Ext.menu.TextItem.superclass.constructor.call(this)};Ext.extend(Ext.menu.TextItem,Ext.menu.BaseItem,{hideOnClick:false,itemCls:"x-menu-text",onRender:function(){var A=document.createElement("span");A.className=this.itemCls;A.innerHTML=this.text;this.el=A;Ext.menu.TextItem.superclass.onRender.apply(this,arguments)}});