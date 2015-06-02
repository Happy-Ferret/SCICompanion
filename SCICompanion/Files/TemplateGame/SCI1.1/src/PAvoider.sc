(version 2)
(include "sci.sh")
(use "PolyPath")
(use "Polygon")
(use "Obj")
(script 927)



(procedure (localproc_042b param1)
    (var temp0, temp1[2], temp3, temp4)
    = temp3 -100
    = temp0 0
    (while (<> temp3 30583)
        = temp3 proc999_6(param1 (* 2 temp0))
        ++temp0
    )
    return --temp0
)


(procedure (localproc_0454 param1)
    (var temp0, temp1, temp2, temp3)
    = temp3 (send param1:size)
    = temp0 0
    (while (< temp0 temp3)
        = temp1 (send param1:at(temp0))
        = temp2 (send temp1:type)
        (if (>= temp2 16)
            (send temp1:type((- temp2 16)))
        )
        ++temp0
    )
)


(class PAvoider of Code
    (properties
        client 0
        oldBlocker 0
        oldBlockerMover 0
        oldMoverX -99
        oldMoverY -99
    )

    (method (init theClient)
        (if (>= paramTotal 1)
            = client theClient
        )
    )


    (method (doit)
        (var temp0, temp1, temp2, temp3, theOldBlocker, temp5, temp6, temp7, temp8, clientMover, temp10[4], temp14, newPolygon, temp16, temp17, temp18[5], clientHeading)
        = clientMover (send client:mover)
        (if (oldBlocker and (>= (send client:distanceTo(oldBlocker)) 20))
            (send oldBlocker:ignoreActors(0))
            (if (oldBlockerMover)
                (send oldBlocker:mover(oldBlockerMover))
            )
            = oldMoverY (= oldMoverX -99)
            = oldBlocker (= oldBlockerMover 0)
            (if ((clientMover and IsObject((send clientMover:obstacles))) and (send ((send clientMover:obstacles)):isEmpty()))
                (send ((send clientMover:obstacles)):dispose())
                (send clientMover:obstacles(0))
            )
        )
        = clientMover (send client:mover)
        (if (((clientMover and IsObject((= theOldBlocker (send ((= clientMover (send client:mover))):doit())))) and not (send clientMover:completed)) and (send clientMover:isKindOf(PolyPath)))
            (if ((send theOldBlocker:respondsTo(#mover)))
                = oldBlockerMover (send theOldBlocker:mover)
                (if (oldBlockerMover)
                    (send theOldBlocker:mover(0))
                )
            )(else
                = oldBlockerMover 0
            )
            = oldMoverX (send clientMover:finalX)
            = oldMoverY (send clientMover:finalY)
            (send (= oldBlocker theOldBlocker):ignoreActors(1))
            = temp2 (+ (* 2 (send client:xStep)) (/ Max(CelWide((send client:view) 2 0) CelWide((send client:view) 0 0)) 2))
            = temp5 (- (send theOldBlocker:brLeft) temp2)
            = temp6 CoordPri(1 CoordPri((send theOldBlocker:y)))
            = temp3 (* 2 (send theOldBlocker:yStep))
            = temp7 (+ (send theOldBlocker:brRight) temp2)
            = temp8 (+ (+ (send theOldBlocker:y) temp3) 2)
            (if (<= (- temp8 temp6) 3)
                = temp6 (- temp6 2)
                = temp8 (+ temp8 2)
            )
            = temp0 (- (send clientMover:finalX) (send client:x))
            = temp1 (- (send clientMover:finalY) (send client:y))
            = clientHeading (send client:heading)
            (if ((<= 85 clientHeading) and (<= (= clientHeading (send client:heading)) 95))
                = temp14 0
            )(else
                (if ((<= 265 clientHeading) and (<= clientHeading 275))
                    = temp14 1
                )(else
                    (if (>= temp1 0)
                        = temp14 2
                    )(else
                        = temp14 3
                    )
                )
            )
            (switch (temp14)
                (case 3
                    = temp17 (send ((Polygon:new())):
                            init(temp5 (send client:y) temp5 temp6 temp7 temp6 temp7 (send client:y) 30583 0)
                            type(2)
                            name("isBlockedPoly")
                            yourself()
                        )
                )
                (case 2
                    = temp17 (send ((Polygon:new())):
                            init(temp7 (send client:y) temp7 temp8 temp5 temp8 temp5 (send client:y) 30583 0)
                            type(2)
                            name("isBlockedPoly")
                            yourself()
                        )
                )
                (case 0
                    = temp17 (send ((Polygon:new())):
                            init((send client:x) temp6 temp7 temp6 temp7 temp8 (send client:x) temp8 30583 0)
                            type(2)
                            name("isBlockedPoly")
                            yourself()
                        )
                )
                (case 1
                    = temp17 (send ((Polygon:new())):
                            init((send client:x) temp8 temp5 temp8 temp5 temp6 (send client:x) temp6 30583 0)
                            type(2)
                            name("isBlockedPoly")
                            yourself()
                        )
                )
            )
            (if (not (send clientMover:obstacles))
                (send clientMover:obstacles((List:new())))
            )
            = temp16 MergePoly((send temp17:points) (send ((send clientMover:obstacles)):elements) (send ((send clientMover:obstacles)):size))
            (if (temp16)
                = newPolygon (Polygon:new())
                (send newPolygon:
                    points(newPolygon)
                    size(localproc_042b(newPolygon))
                    type(2)
                    dynamic(1)
                )
            )
            (send ((send clientMover:obstacles)):add(newPolygon))
            (send clientMover:
                value(2)
                init(client (send clientMover:finalX) (send clientMover:finalY))
            )
            (send ((send clientMover:obstacles)):delete(newPolygon))
            (send ((send clientMover:obstacles)):delete(temp17))
            (if (IsObject((send clientMover:obstacles)))
                localproc_0454((send clientMover:obstacles))
            )
            (send temp17:dispose())
            (send newPolygon:dispose())
        )
    )


    (method (dispose)
        (if (IsObject(oldBlockerMover))
            (send oldBlockerMover:dispose())
        )
        (super:dispose())
    )

)
