(define (problem assem-x-13)
   (:domain assembly)
   (:objects frob sprocket-32 hack-29 fastener-31 plug-14
             whatsis-9 contraption-10 tube-11 connector-12 foobar-13
             mount-7 device-15 thingumbob-16 widget-5 doodad-6 plug coil-8
             gimcrack-3 valve-4 socket-1 hoozawhatsie-2 device whatsis
             sprocket kludge valve wire contraption widget thingumbob mount
             foobar socket hoozawhatsie hack unit - assembly
             voltmeter - resource)
   (:init (available hack-29)
          (available fastener-31)
          (available whatsis-9)
          (available contraption-10)
          (available tube-11)
          (available connector-12)
          (available foobar-13)
          (available device-15)
          (available thingumbob-16)
          (available widget-5)
          (available doodad-6)
          (available coil-8)
          (available gimcrack-3)
          (available valve-4)
          (available socket-1)
          (available hoozawhatsie-2)
          (available device)
          (available sprocket)
          (available valve)
          (available wire)
          (available contraption)
          (available thingumbob)
          (available foobar)
          (available socket)
          (available hoozawhatsie)
          (available hack)
          (available unit)
          (available voltmeter)
          (requires plug-14 voltmeter)
          (requires mount-7 voltmeter)
          (requires plug voltmeter)
          (requires whatsis voltmeter)
          (requires widget voltmeter)
          (requires mount voltmeter)
          (part-of sprocket-32 frob)
          (part-of plug-14 frob)
          (part-of mount-7 frob)
          (part-of plug frob)
          (part-of whatsis frob)
          (part-of widget frob)
          (part-of mount frob)
          (part-of hack-29 sprocket-32)
          (part-of socket sprocket-32)
          (part-of fastener-31 sprocket-32)
          (part-of whatsis-9 plug-14)
          (transient-part contraption-10 plug-14)
          (part-of tube-11 plug-14)
          (part-of connector-12 plug-14)
          (part-of foobar-13 plug-14)
          (transient-part device-15 mount-7)
          (part-of thingumbob-16 mount-7)
          (part-of widget-5 mount-7)
          (part-of doodad-6 mount-7)
          (transient-part coil-8 plug)
          (part-of gimcrack-3 plug)
          (transient-part valve-4 plug)
          (part-of socket-1 plug)
          (transient-part hoozawhatsie-2 plug)
          (part-of device plug)
          (part-of sprocket whatsis)
          (part-of device-15 whatsis)
          (part-of kludge whatsis)
          (part-of valve kludge)
          (part-of wire kludge)
          (part-of contraption kludge)
          (part-of thingumbob widget)
          (part-of valve-4 widget)
          (part-of coil-8 widget)
          (part-of foobar mount)
          (transient-part socket mount)
          (part-of hoozawhatsie-2 mount)
          (part-of hoozawhatsie mount)
          (part-of hack mount)
          (part-of unit mount)
          (assemble-order sprocket-32 whatsis frob)
          (assemble-order plug-14 mount-7 frob)
          (assemble-order plug mount-7 frob)
          (assemble-order plug sprocket-32 frob)
          (assemble-order socket hoozawhatsie-2 sprocket-32)
          (assemble-order contraption-10 whatsis-9 plug-14)
          (remove-order whatsis-9 contraption-10 plug-14)
          (assemble-order device-15 doodad-6 mount-7)
          (remove-order doodad-6 device-15 mount-7)
          (assemble-order widget-5 device-15 mount-7)
          (assemble-order widget-5 doodad-6 mount-7)
          (assemble-order coil-8 hoozawhatsie-2 plug)
          (assemble-order coil-8 gimcrack-3 plug)
          (remove-order hoozawhatsie-2 coil-8 plug)
          (assemble-order valve-4 gimcrack-3 plug)
          (remove-order gimcrack-3 valve-4 plug)
          (assemble-order socket-1 coil-8 plug)
          (assemble-order hoozawhatsie-2 gimcrack-3 plug)
          (remove-order gimcrack-3 hoozawhatsie-2 plug)
          (assemble-order device hoozawhatsie-2 plug)
          (assemble-order device-15 doodad-6 whatsis)
          (assemble-order contraption valve kludge)
          (assemble-order valve-4 gimcrack-3 widget)
          (assemble-order coil-8 hoozawhatsie-2 widget)
          (assemble-order coil-8 gimcrack-3 widget)
          (assemble-order socket hoozawhatsie-2 mount)
          (remove-order tube socket mount)
          (assemble-order hoozawhatsie-2 gimcrack-3 mount))
   (:goal (complete frob)))