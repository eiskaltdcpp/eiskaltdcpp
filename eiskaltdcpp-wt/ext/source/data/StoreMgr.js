/*
 * Ext JS Library 2.0.2
 * Copyright(c) 2006-2008, Ext JS, LLC.
 * licensing@extjs.com
 * 
 * http://extjs.com/license
 */

/**
 * @class Ext.StoreMgr
 * @extends Ext.util.MixedCollection
 * The default global group of stores.
 * @singleton
 */
Ext.StoreMgr = Ext.apply(new Ext.util.MixedCollection(), {
    register : function(){
        for(var i = 0, s; s = arguments[i]; i++){
            this.add(s);
        }
    },

    unregister : function(){
        for(var i = 0, s; s = arguments[i]; i++){
            this.remove(this.lookup(s));
        }
    },

    /**
     * Gets a registered Store by id
     * @param {String/Object} id The id of the Store or a Store
     * @return {Ext.data.Store}
     */
    lookup : function(id){
        return typeof id == "object" ? id : this.get(id);
    },

    // getKey implementation for MixedCollection
    getKey : function(o){
         return o.storeId || o.id; 
    }
});