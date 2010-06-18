/*
 * Ext JS Library 2.0.2
 * Copyright(c) 2006-2008, Ext JS, LLC.
 * licensing@extjs.com
 * 
 * http://extjs.com/license
 */

Ext.LayoutStateManager=function(A){this.state={north:{},south:{},east:{},west:{}}};Ext.LayoutStateManager.prototype={init:function(D,G){this.provider=G;var F=G.get(D.id+"-layout-state");if(F){var E=D.isUpdating();if(!E){D.beginUpdate()}for(var A in F){if(typeof F[A]!="function"){var B=F[A];var C=D.getRegion(A);if(C&&B){if(B.size){C.resizeTo(B.size)}if(B.collapsed==true){C.collapse(true)}else{C.expand(null,true)}}}}if(!E){D.endUpdate()}this.state=F}this.layout=D;D.on("regionresized",this.onRegionResized,this);D.on("regioncollapsed",this.onRegionCollapsed,this);D.on("regionexpanded",this.onRegionExpanded,this)},storeState:function(){this.provider.set(this.layout.id+"-layout-state",this.state)},onRegionResized:function(B,A){this.state[B.getPosition()].size=A;this.storeState()},onRegionCollapsed:function(A){this.state[A.getPosition()].collapsed=true;this.storeState()},onRegionExpanded:function(A){this.state[A.getPosition()].collapsed=false;this.storeState()}};