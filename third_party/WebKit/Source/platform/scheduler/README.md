# Blink Scheduler

This directory contains the Blink Scheduler, which coordinates task execution
in renderer processes. The main subdirectories are:

- `base/` -- basic scheduling primitives such as `TaskQueue` and
   `TaskQueueManager`.
- `child/` -- contains the `ChildScheduler` which is the base class for all
   thread schedulers, as well as a `WorkerScheduler` for worker threads.
- `utility/` -- a small scheduler for utility processes.
- `renderer/` -- `RendererScheduler` for the renderer process.

The scheduler exposes an API at `public/platform/scheduler`.

# Documentation

The following is a collection of scheduling-related documentation about the
Blink Scheduler as well as other schedulers in Chrome.


## 2017
* [Improved load time scheduling](https://docs.google.com/document/d/1q5uPIKyUP0X7KaQRyxWXmIzMvKF3fx1j6QPCWhjI82o/edit)
* [Wake-up based throttling](https://docs.google.com/document/d/1A87Ci3_USDyQEdlmXTO1spQxUcR_ML5zqiCsaow4NGM/edit)
* [Background tabs & offscreen frames](https://docs.google.com/document/d/18_sX-KGRaHcV3xe5Xk_l6NNwXoxm-23IOepgMx4OlE4/edit)
* [BeginFrame sequence numbers + acknowledgements](https://docs.google.com/document/d/1nxaunQ0cYWxhtS6Zzfwa99nae74F7gxanbuT5JRpI6Y/edit)
* [Background tab use cases](https://docs.google.com/document/d/16-QGneIkYNbNleoXbdD-mRMYdZAG2JIjMcTVxSC3ZWc/edit)
* [Activity traits](https://docs.google.com/document/d/1BaJpx08vbPz_1LCj9tehnatZNqh1eLPeE9xoUnWdlW4/edit#heading=h.nwhgpfhlxswr)

## 2016

* [Time-based renderer task throttling](https://drive.google.com/open?id=1vCUeGfr2xzZ67SFt2yZjNeaIcXGp2Td6KHN7bI02ySo)
* [V8 Performance Mode](https://drive.google.com/open?id=1bRVAP08qNBvnEm_vO4hW1-NqQC9-lQZjUH29_vwfYRY)
* [Isolating performance of third-party iframes](https://docs.google.com/document/d/1CEggurHQGXenhu_GQT7KnRvtSuowuenXpxVzYSeRxSY/edit)
* [Folly of Scheduling (BlinkOn 6)](https://drive.google.com/open?id=1ZMxbnSn1R1o2-NGztP0mVyOOoQg24bLSqWE1SWXnQ_E)
* [Rendering pipeline throttling (BlinkOn 6)](https://docs.google.com/presentation/d/1aPZzH7J0O29sqA_FzsuWQNDwK6CoNcAcpMvJexsO6Vg/edit)
* [Power usage impact of render pipeline throttling](https://docs.google.com/document/d/1jMuvRYWptZfP5zpvWmPJPRL-iowtgBVX45rSvew0VH4)
* [The future of TaskRunnerHandles](https://docs.google.com/document/d/1A_LRKyTOCzhRPOY4Q3RsePuw4UCsvxuFYx6D18BaYk0/edit#heading=h.xgjl2srtytjt)
* [Improved policy for blocking expensive tasks](https://docs.google.com/document/d/14VdbqN-ehgpNC4KYVpPQFiQpfxOQiVtJgYjXUJGI4f0/edit#)
* [scheduler-dev performance metrics](https://docs.google.com/document/d/15CIJ4eMnwOneshhjFxVjz3FCV7ja9lrlQOEZGWLZdgA/edit)
* [FrameBlamer](https://docs.google.com/document/d/15BB-suCb9j-nFt55yCFJBJCGzLg2qUm3WaSOPb8APtI/edit)
* [Virtual time in Blink](https://drive.google.com/open?id=1y9KDT_ZEzT7pBeY6uzVt1dgKlwc1OB_vY4NZO1zBQmo)
* [Browser I/O scheduler (Lucky Luke)](https://docs.google.com/document/d/1S2AAeoo1xa_vsLbDYBsDHCqhrkfiMgoIPlyRi6kxa5k/edit)

## 2015

* [Virtual time in Headless Chrome](https://docs.google.com/document/d/1dIMHIl1xutUXqXWRXqXrDd3bo9hachIt_ZkPK_BshUs/edit)
* [Task traits](https://docs.google.com/document/d/1d6t7CTobtXLj1gXiBE8SVl_fxJjEazATxYHYGp5ppvE)
* [Compositor and Display Scheduling presentation from scheduling summit](https://docs.google.com/presentation/d/1FpTy5DpIGKt8r2t785y6yrHETkg8v7JfJ26zUxaNDUg/edit?usp=sharing)
* [Modernizing task management in Chrome](https://docs.google.com/document/d/14Q22SKrhN9OykJgRLcHMQVzeXKVJCbx7xQvOeOoNQm4/edit#heading=h.rptt48atxb4e)
* [Outline of known work needed to fix resize](https://docs.google.com/a/chromium.org/document/d/1POLDq-L_T9iZ_Ul39sjOMiOO-yvLnmb1WFsH4JfIyVU/edit?usp=sharing_eid)
* [Blink spatial scheduling](https://docs.google.com/document/d/1k9fL01wwRliVzZW_ibPT8-B9BADz7I87hpFoXFG37aI/edit?pli=1#)
* [Throttling Blink's rendering pipeline for hidden content](https://docs.google.com/document/d/1Dd4qi1b_iX-OCZpelvXxizjq6dDJ76XNtk37SZEoTYQ/edit)
* [Scheduling to avoid checkerboard in Chrome](https://docs.google.com/document/d/1OLp7x06CjBY-0J3TBQzXw8sALHznIx4rYixvnBTbUUA/edit#heading=h.9i2v5u7um22b)
* [State of GPU scheduling](https://docs.google.com/document/d/15gbHgXPyhSlNu1Ku9HF-of8BNOpvsziV1F8IE-kRax4/edit#heading=h.jermw4ib9rwc)
* [Scheduling architecture diagram](https://docs.google.com/drawings/d/1xcHpqhdcIsX0b_sGPuU1SZCsY87UTqJ67qvvbF29oLM/edit)
* [Blink Scheduler-dev BrownBag presentation](https://docs.google.com/presentation/d/1uda_oY0XGMsTsDt9xuWv_eWHWpt_F9VRuL6-5XqBUVY/edit#slide=id.g5e1a23a4d_2274)
* [Event dispatch diagram](https://docs.google.com/drawings/d/1bUukRm-DV34sM7rL2_bSdxaQkZVMQ_5vOa7nzDnmnx8/edit)
* [Proxying MessageLoop tasks to the Scheduler](https://docs.google.com/a/chromium.org/document/d/1qxdh2I61_aB_Uzh1QgNqvdWFBCL_E65G2smoSySw7KU/edit#heading=h.vit0krths7ns)
* [Scheduling JS timer execution](https://docs.google.com/a/chromium.org/document/d/163ow-1wjd6L0rAN3V_U6t12eqVkq4mXDDjVaA4OuvCA/edit?usp=sharing_eid)
* [PSA: How to write reliable layout tests](https://docs.google.com/a/chromium.org/document/d/1Yl4SnTLBWmY1O99_BTtQvuoffP8YM9HZx2YPkEsaduQ/edit#heading=h.ui2te0d6ongo)
* [Long Idle Tasks: Coupling wagons to the Blink Midnight Train](https://docs.google.com/a/chromium.org/document/d/1yBlUdYW8VTIfB-DqhvQqUeP0kf-Ap1W4cao2yQq58Do/edit?pli=1#heading=h.g9y2fheuia8t)
* [Cooperative scheduling in Javascript](https://docs.google.com/a/chromium.org/document/d/1Jb0DRcIeHHFldlI8wkQJ4uAyTZLzNOvH161VBJUF_Oc/edit#)

## 2014

* [Blink Scheduler talk at BlinkOn 3](https://docs.google.com/presentation/d/1V09Qq08_jOucvOFs-C7P4Hz2Vsswa6imqLxAf7ONomQ/edit#slide=id.p)
* [Blink Scheduler](https://docs.google.com/a/chromium.org/document/d/11N2WTV3M0IkZ-kQlKWlBcwkOkKTCuLXGVNylK5E2zvc/edit#heading=h.3ay9sj44f0zd)
* [Blink Scheduler refactoring](https://docs.google.com/a/chromium.org/document/d/16f_RIhZa47uEK_OdtTgzWdRU0RFMTQWMpEWyWXIpXUo/edit#) (moving from Blink to content)
* [Idle Tasks in the Blink Scheduler](https://docs.google.com/a/chromium.org/document/d/1bXcZ45iCr9NPP6UDbY57RCKgSndgaBt9tSgwxV0sg1o/edit)
* [Resource loading tasks and the Blink Scheduler](https://docs.google.com/a/chromium.org/document/d/1kLdtb718AEetE64gL-MmM0YRh7kAkxPpWDRa7OI-scI/edit?usp=sharing_eid)
* [Blink Scheduler friendly HTMLDocumentParser](https://docs.google.com/a/chromium.org/document/d/1Ofil50mhU9IuDkmEdbde18uxquA3WsEdI3vCyYzzDyc/edit#heading=h.fr9ldspsaw6g)
* [Trustable Future Sync Points](https://docs.google.com/document/d/1qqu8c5Gp1faY-AY4CgER-GKs0w7GXlR5YJ-BaIZ4auo/edit?usp=sharing)
* [Unified VSync Scheduling](https://docs.google.com/document/d/13xtO-_NSSnNZRRS1Xq3xGNKZawKc8HQxOid5boBUyX8/edit?usp=sharing)
* [VSync-Aligned Buffered Input](https://docs.google.com/document/d/1L2JTgYMksmXgujKxxhyV45xL8jNhbCh60NQHoueKyS4/edit?usp=sharing)
* [London Perf Summit - Chrome Scheduling](https://docs.google.com/presentation/d/1I105Uk7nlH_Kj4UaqC7Ygkw3eNuDINQXPtYYSusW8Ho/edit?usp=sharing)
* [GPU Service Scheduling Latency](https://docs.google.com/document/d/1hjVckIpb9WBE7A9HUxAmutRJEgSkZO_JAFfgwOE-8NE/edit?usp=sharing)
* Related
  * [Sync Point Shader Arguments](https://docs.google.com/document/d/1GlnjZI0jDNPXZIlhdcU135BGnsP7T3WY0UR4IwjEILU/edit?usp=sharing)
  * [Asynchronous Shader Input from the CPU](https://docs.google.com/document/d/1daXOSiYUHvDcG5dR9OUQWx6rllW127mRcxiVUTK9DdM/edit?usp=sharing)
  * [T-Sync Display Interface](https://docs.google.com/document/d/1ZoH6a-Pxsnh9Xu_2rtF5jss2d352Klpu_23urKghaH0/edit?usp=sharing)
  * [Surfaces](https://docs.google.com/a/chromium.org/document/d/1RxbffpK_GxPtZscXgIEN0N9ZT7IC8BObnbx9ynw92qg/edit?pli=1)
  * [Better Video Frame Scheduling](https://docs.google.com/a/chromium.org/document/d/1xauQd5Tt2MuM82MAwIqIW7IEkVj4VnjWBB-zUODfERQ)

## 2013

* [Synthetic Scheduler Tests](https://docs.google.com/a/chromium.org/document/d/17yhE5Po9By0sCdM1yZT3LiUECaUr_94rQt9j-4tOQIM)
* [Chrome Frame Synchronization](https://docs.google.com/presentation/d/1q2WU0LusCyQFKDMjOSWLj3xGeOxMWmLzConrC8euJpA/edit?usp=sharing)
* [Chrome Scheduling Overhaul Phase 1](https://docs.google.com/document/d/1LUFA8MDpJcDHE0_L2EHvrcwqOMJhzl5dqb0AlBSqHOY/edit?usp=sharing)
* [Chrome Scheduling Overhaul Phase 2](https://docs.google.com/document/d/1VJf2busac85FRQYXhn8hdc-x4yp77JUroTrY-_sj5Ck/edit?usp=sharing)
* [Improved vsync scheduling for Chrome on Android](https://docs.google.com/a/chromium.org/document/d/16822du6DLKDZ1vQVNWI3gDVYoSqCSezgEmWZ0arvkP8/edit)
* [ZilCh](https://docs.google.com/document/d/1HmS0YQtWg2ToY67fE8A33PJUyPSwGUwUCLMk_zjK7ik/edit?usp=sharing)

## Miscellaneous

* [Rendering for "Glass Time"](https://docs.google.com/a/google.com/presentation/d/1oKEunkaeiTwznGaIX_yIhe6HPfZBXhtv8r5J5hz52UI/edit#slide=id.g2b8380fec_0129)
* [Motion Vectors: An interface between Render and Input Systems](http://www.google.com/url?q=http%3A%2F%2Fgo%2Finput-motion-vectors&sa=D&sntz=1&usg=AFQjCNF0sC31c9FLCscR8HtXiz_kP5EaPw)
* [Begin Frame / VSync Design Diagram](https://docs.google.com/a/chromium.org/drawings/d/1WEj-6A-8FmJNIMbd9hvkvxAuOOTwQvkSjbKR79YCt-c/edit)
