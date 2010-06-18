/*
 * Ext JS Library 2.0.2
 * Copyright(c) 2006-2008, Ext JS, LLC.
 * licensing@extjs.com
 * 
 * http://extjs.com/license
 */

Ext.ReaderLayout=function(B,C){var D=B||{size:{}};Ext.ReaderLayout.superclass.constructor.call(this,C||document.body,{north:D.north!==false?Ext.apply({split:false,initialSize:32,titlebar:false},D.north):false,west:D.west!==false?Ext.apply({split:true,initialSize:200,minSize:175,maxSize:400,titlebar:true,collapsible:true,animate:true,margins:{left:5,right:0,bottom:5,top:5},cmargins:{left:5,right:5,bottom:5,top:5}},D.west):false,east:D.east!==false?Ext.apply({split:true,initialSize:200,minSize:175,maxSize:400,titlebar:true,collapsible:true,animate:true,margins:{left:0,right:5,bottom:5,top:5},cmargins:{left:5,right:5,bottom:5,top:5}},D.east):false,center:Ext.apply({tabPosition:"top",autoScroll:false,closeOnTab:true,titlebar:false,margins:{left:D.west!==false?0:5,right:D.east!==false?0:5,bottom:5,top:2}},D.center)});this.el.addClass("x-reader");this.beginUpdate();var A=new Ext.BorderLayout(Ext.getBody().createChild(),{south:D.preview!==false?Ext.apply({split:true,initialSize:200,minSize:100,autoScroll:true,collapsible:true,titlebar:true,cmargins:{top:5,left:0,right:0,bottom:0}},D.preview):false,center:Ext.apply({autoScroll:false,titlebar:false,minHeight:200},D.listView)});this.add("center",new Ext.NestedLayoutPanel(A,Ext.apply({title:D.mainTitle||"",tabTip:""},D.innerPanelCfg)));this.endUpdate();this.regions.preview=A.getRegion("south");this.regions.listView=A.getRegion("center")};Ext.extend(Ext.ReaderLayout,Ext.BorderLayout);