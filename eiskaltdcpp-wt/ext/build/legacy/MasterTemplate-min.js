/*
 * Ext JS Library 2.0.2
 * Copyright(c) 2006-2008, Ext JS, LLC.
 * licensing@extjs.com
 * 
 * http://extjs.com/license
 */

Ext.MasterTemplate=function(){Ext.MasterTemplate.superclass.constructor.apply(this,arguments);this.originalHtml=this.html;var D={};var A,E=this.subTemplateRe;E.lastIndex=0;var C=0;while(A=E.exec(this.html)){var B=A[1],F=A[2];D[C]={name:B,index:C,buffer:[],tpl:new Ext.Template(F)};if(B){D[B]=D[C]}D[C].tpl.compile();D[C].tpl.call=this.call.createDelegate(this);C++}this.subCount=C;this.subs=D};Ext.extend(Ext.MasterTemplate,Ext.Template,{subTemplateRe:/<tpl(?:\sname="([\w-]+)")?>((?:.|\n)*?)<\/tpl>/gi,add:function(B,A){if(arguments.length==1){A=arguments[0];B=0}var C=this.subs[B];C.buffer[C.buffer.length]=C.tpl.apply(A);return this},fill:function(D,C,F){var B=arguments;if(B.length==1||(B.length==2&&typeof B[1]=="boolean")){C=B[0];D=0;F=B[1]}if(F){this.reset()}for(var E=0,A=C.length;E<A;E++){this.add(D,C[E])}return this},reset:function(){var B=this.subs;for(var A=0;A<this.subCount;A++){B[A].buffer=[]}return this},applyTemplate:function(A){var B=this.subs;var C=-1;this.html=this.originalHtml.replace(this.subTemplateRe,function(D,E){return B[++C].buffer.join("")});return Ext.MasterTemplate.superclass.applyTemplate.call(this,A)},apply:function(){return this.applyTemplate.apply(this,arguments)},compile:function(){return this}});Ext.MasterTemplate.prototype.addAll=Ext.MasterTemplate.prototype.fill;Ext.MasterTemplate.from=function(B,A){B=Ext.getDom(B);return new Ext.MasterTemplate(B.value||B.innerHTML,A||"")};