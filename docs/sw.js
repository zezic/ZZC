importScripts('/_nuxt/workbox.4c4f5ca6.js')

workbox.precaching.precacheAndRoute([
  {
    "url": "/_nuxt/49fcb0bd830d48cb7250.js",
    "revision": "be3fb4414254ac127cd55d60b3814086"
  },
  {
    "url": "/_nuxt/576dafe90a7ea5e15d34.js",
    "revision": "436720009724342d4680d2ed2286e3d9"
  },
  {
    "url": "/_nuxt/71c3b98793a3d58e3882.js",
    "revision": "19fba1d05b6b68e5fb345c1eea9f12f2"
  },
  {
    "url": "/_nuxt/d3e22f2ef2a7d1ea287c.js",
    "revision": "408a9bf036e4f4e5cc314c111c18b9ea"
  },
  {
    "url": "/_nuxt/d4c71490175508f48d4f.js",
    "revision": "6339319a4bf037c7bbf490283ce3198e"
  },
  {
    "url": "/_nuxt/e62b97a87024cc92d052.js",
    "revision": "b7e3bbc8df509376d6712be1896cba05"
  },
  {
    "url": "/_nuxt/ecfcf50faa94c5ef1bb4.js",
    "revision": "91462054d36e7faa7882f7769ec1e470"
  },
  {
    "url": "/_nuxt/f59f9928cd92c8a9eff3.js",
    "revision": "2836f139b8f2a9f7d4fa82589c95ae54"
  },
  {
    "url": "/_nuxt/f7ae4c6eda29ca03adff.js",
    "revision": "3995b3cb745cf3719559780a33c9b7b9"
  }
], {
  "cacheId": "zzc-docs",
  "directoryIndex": "/",
  "cleanUrls": false
})

workbox.clientsClaim()
workbox.skipWaiting()

workbox.routing.registerRoute(new RegExp('/_nuxt/.*'), workbox.strategies.cacheFirst({}), 'GET')

workbox.routing.registerRoute(new RegExp('/.*'), workbox.strategies.networkFirst({}), 'GET')
