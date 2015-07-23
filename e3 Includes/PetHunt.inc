#event doPetHunt "<#1#> Pet Hunter On#*#"
SUB event_doPetHunt(line, ChatSender)
	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |--------Included Bot-----------|    |-----------In Zone-------------|
   /if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]}) {
      /echo start
      /declare doHunt              bool outer
      /varset doHunt True
      /call startPetHunter

   }
/return

#event stopPetHunt "<#1#> Pet Hunter Off#*#"
SUB event_stopPetHunt(line, ChatSender)
	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |--------Included Bot-----------|    |-----------In Zone-------------|
   /if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]}) {
      /varset doHunt False
      /echo Hunt Stopped
   }
/return


Sub startPetHunter
   |------------------------------------------------------------
   |How many times should aquire target fail before delaying?
   |------------------------------------------------------------
   /declare RV_FailMax          int outer  3
   |------------------------------------------------------------
   |How far would you like to target a mob?
   |------------------------------------------------------------
   /declare RV_MaxRadius        int outer  160
   |------------------------------------------------------------
   |How far is the combat range?
   |------------------------------------------------------------
   /declare RV_Range            int outer 160
   |------------------------------------------------------------
   |What is the minimum Z Value of mobs I should target?
   |------------------------------------------------------------
   /declare RV_MinZRange        int outer  -1000
   |------------------------------------------------------------
   |What is the maximum Z Value of mobs I should target?
   |------------------------------------------------------------
   /declare RV_MaxZRange        int outer  1000
   |------------------------------------------------------------
   |Should I loot all items?
   |------------------------------------------------------------
   /declare RV_LootAllItems     int outer  0
   |------------------------------------------------------------
   |Should I display stats?
   |------------------------------------------------------------
   /declare RV_DisplayStats     int outer  1

   |------------------------------------------------------------
   |Loot Array Information.
   |------------------------------------------------------------
   /call ReadINI "e3 Macro Inis\PetHunter.ini" "${Zone.Name}" Mob
   /if (!${Defined[RV_MobArray]}) {
      /echo Mob Array Creation Error, ending macro...
      /endmacro
   }



   |------------------------------------------------------------
   |Variables that you don't need to worry about.
   |------------------------------------------------------------
   /declare RV_FailCounter      int outer  0
   /declare RV_MyTargetID       int outer  0
   /declare RV_MyTargetName     string outer
   /declare RV_MyTargetDead     int outer  0
   /declare RV_InvalidTargetID  int outer  0
   /declare RV_HasTarget        int outer  0
   /declare RV_RandomWait       int outer  0
   /declare RV_LootSlot         int outer  0
   /declare RV_CheckLook        int outer  0
   /declare RV_Fighting         int outer  0
   /declare RV_TargetDead       int outer  0
   /declare RV_MyXLOC           int outer  0
   /declare RV_MyYLOC           int outer  0
   /declare RV_MyNewXLOC        int outer  0
   /declare RV_MyNewYLOC        int outer  0
   /declare SetLocNumber        int outer  1
   /declare MyLocArray[6,6]       string outer

   /declare RV_FastRange        int outer
   /declare RV_RangeMax         int outer
   /declare RV_RangeMin         int outer
   /varcalc RV_FastRange ${RV_Range}+3
   /varcalc RV_RangeMax ${RV_Range}+1
   /varcalc RV_RangeMin ${RV_Range}-1
   /call BuildArray SavedLoc "-157,0"
   /call BuildArray SavedLoc "24,79"
   /call BuildArray SavedLoc "147,87"
   /call BuildArray SavedLoc "-110,-13"
   /call BuildArray SavedLoc "86,-15"
   /call BuildArray SavedLoc "15,-49"
   /call BuildArray SaveLoc ""
   :Start
   /echo Main Loop
   /if (!${Me.PetBuff["Moon Curse Rk. I"]}) {
      /bct Beerstand //casting "Moon Curse Rk. I" -targetid|${Spawn[pet ${Spawn[id ${Me.ID}].Pet}].ID}
      /delay 5s
      /goto :Start
   }
   /if (!${Me.PetBuff["Blessing of Oak"]}) {
      /bct Beerstand //casting "Blessing of Oak"  -targetid|${Spawn[pet ${Spawn[id ${Me.ID}].Pet}].ID}
      /delay 5s
      /goto :Start
   }
   /if (!${Me.PetBuff["Redwood Skin Rk. I"]}) {
      /bct Beerstand //casting "Redwood Skin Rk. I" -targetid|${Spawn[pet ${Spawn[id ${Me.ID}].Pet}].ID}
      /delay 5s
      /goto :Start
   }
   /if (!${Me.PetBuff["Balikor's Mark"]}) {
      /bct Ninetoe //casting "Balikor's Mark" -targetid|${Spawn[pet ${Spawn[id ${Me.ID}].Pet}].ID}
      /delay 2s
      /goto :Start
   }
   /if (!${Me.PetBuff["Elemental Fury"]}) {
      /call e3_Cast ${Me..Pet.ID} "Elemental Fury"
      /delay 2s
      /goto :Start
   }
   /if (${Me.Pet.PctHPs} < ${60}) {
      /bc Pet Below 60% HP
      /tell ninetoe PEPet
      /goto :Start
   }
   /doevents
   /call GMCheck
   /call GetTarget

   :KillAdds
   /if (${Me.Pet.PctHPs} < ${60}) {
      /tell ninetoe PEPet
      /goto :KillAdds
   }
   /if (${RV_HasTarget}) /call CombatSub
   /call ResetSub

   /varset RV_RandomWait ${Math.Rand[5]}
   /varcalc RV_RandomWait ${RV_RandomWait}+1
   /echo Paranoia - Waiting 1 seconds before resuming
   /delay 1s

   /if (${Target.ID}) && (${doHunt}) {
      /echo Looks like something is attacking us, killing it...
      /delay 1s
      /varset RV_HasTarget 1
      /varset RV_Fighting 1
      /goto :KillAdds
   }
   /if (${doHunt}) /bc New Loop
   /if (${doHunt}) /goto :Start

/return


|--------------------------------------------------------------------------------
|SUB: Aquire Target
|--------------------------------------------------------------------------------
Sub GetTarget

   /declare RV_CurrentRadius   int local
   /declare RV_TargetSub   int local
   :Acquire
   /for RV_CurrentRadius 50 to ${RV_MaxRadius} step 50
      /for RV_TargetSub 1 to ${RV_MobArray.Size}
         |/echo radius ${RV_CurrentRadius}  ${RV_InvalidTargetID} npc ${RV_MobArray[${RV_TargetSub}]}
         /squelch /target radius ${RV_CurrentRadius}  npc ${RV_MobArray[${RV_TargetSub}]}
         /varset RV_MyTargetID ${Target.ID}
         /varset RV_MyTargetDead 0
         /if (${Target.ID}) {
            |**
            /if (${Int[${Target.PctHPs}]}<100) {
               /echo Mob NOT a Full Health, picking another...
               /varset RV_InvalidTargetID ${Target.ID}
               /call ResetSub
               /goto :Acquire
            }
            **|
            /if (${Int[${Target.Z}]}<${RV_MinZRange}) {
               /echo Mob is BELOW Min Z Range, picking another...
               /varset RV_InvalidTargetID ${Target.ID}
               /call ResetSub
               /goto :Acquire
            }
            /if (${Int[${Target.Z}]}>${RV_MaxZRange}) {
               /echo Mob is ABOVE Max Z Range, picking another...
               /varset RV_InvalidTargetID ${Target.ID}
               /call ResetSub
               /goto :Acquire
            }
            /if (${Target.ID}==${Me.Pet.ID}) {
              /call ResetSub
              /goto :Acquire
              /varset RV_InvalidTargetID ${Target.ID}
            }
            /call Triangulate_Distance ${Target.Loc} ${Me.Loc.Replace[ ,]}
            /echo mop is ${Macro.Return} distance
            /if (${Macro.Return} > ${RV_MaxRadius})  {
               /bc Mob out of Range getting another
               /stick on
               /delay 1s
               /stick off


            }
            /varset RV_HasTarget 1
            /varset RV_MyTargetName ${Target.CleanName}
            /echo Acquired ${Target.CleanName} at range ${Int[${Target.Distance}]}
            /return
         }
      /next RV_TargetSub
      /delay 2
   /next RV_CurrentRadius

   /if (!${Target.ID}) {

      /delay 2s
      /varcalc RV_FailCounter ${RV_FailCounter}+1
      /echo Failed to Acquire Target in Range ${RV_MaxRadius} ${RV_FailCounter} Time(s)
      /if (${RV_FailCounter}>=${RV_FailMax}) {
         /echo Waiting for Respawns, Resetting Failure Counter...
         /if (${Int[${SetLocNumber}]}==6) {
            /bc resetting route
            /varcalc SetLocNumber 1
         } else {
             /varcalc SetLocNumber ${SetLocNumber}+1
         }
         /bc -MoveTo -Loc|${SavedLoc[${SetLocNumber}]}
         /call MoveTo -Loc|${SavedLoc[${SetLocNumber}]}

         /delay 15s
         /varset RV_FailCounter 0

      }
      /goto :Acquire
   }
/return


|--------------------------------------------------------------------------------
|SUB: Combat
|--------------------------------------------------------------------------------
Sub CombatSub

   /echo Attacking Mob NOW!
   /varset RV_Fighting 1
   /varset RV_TargetDead 0

   :CombatLoop
   /doevents
   /squelch /pet qattack

   /if (!${Target.ID}) {
      /pet back off
      /varset RV_TargetDead 1
      /varset RV_Fighting 0
      /delay 1s
      /delay 1s
      /if (!${Target.ID}) {
         /call ResetSub
         /return
      }
   }
   /if (!${RV_TargetDead}) {
      /if (${doHunt}) /goto :CombatLoop
   }

/return


|--------------------------------------------------------------------------------
|SUB: Reset
|--------------------------------------------------------------------------------
Sub ResetSub

   /keypress esc
   /keypress esc
   /keypress esc
   /keypress esc
   /keypress m
   /varset RV_HasTarget 0
   /varset RV_TargetDead 0
   /varset RV_Fighting 0

/return

|--------------------------------------------------------------------------------
|SUB: Moving
|--------------------------------------------------------------------------------
Sub MoveToMob

   /varset RV_MyXLOC ${Int[${Me.X}]}
   /varset RV_MyYLOC ${Int[${Me.Y}]}
   /declare RV_DistanceTimer timer 15

   /doevents

   :MovementLoop

   /if ((!${RV_Fighting})&&(!${RV_TargetDead})&&(${Target.PctHPs}<100)) {
      /echo Mob not at full health, picking another...
      /varset RV_InvalidTargetID ${Target.ID}
      /varset RV_HasTarget 0
      /call ResetSub
      /return
   }

   /if (${Target.ID}) {
      /face fast
   }

   /if (${Int[${Target.Distance}]}<${RV_FastRange}&&${Int[${Target.Distance}]}>${RV_RangeMax}) {
      /call Triangulate_Distance ${Me.Loc.Replace[ ,]} ${y_loc},${x_loc}

      /call MoveTo -Loc|${RV_MyNewXLOC},${RV_MyNewYLOC.X}
   }
   /if (${Int[${Target.Distance}]}<${RV_RangeMin}) {
      /keypress back
   }
   /if (!${RV_DistanceTimer}) {
      /if ((${RV_MyXLOC}==${Int[${Me.X}]})&&(${RV_MyYLOC}==${Int[${Me.Y}]})) /call HitObstacle
      /varset RV_MyXLOC ${Int[${Me.X}]}
      /varset RV_MyYLOC ${Int[${Me.Y}]}
      /varset RV_DistanceTimer 15
      /goto :Movementloop
   }
   /if (${Int[${Target.Distance}]}>${RV_FastRange}) /goto :MovementLoop

/return
|--------------------------------------------------------------------------------
|SUB: Obstacle Avoidance
|--------------------------------------------------------------------------------
Sub HitObstacle

   /echo Obstacle hit, moving around it...
   /keypress forward
   /keypress back hold
   /delay 3
   /keypress back
   /if (${Math.Rand[100]}+1>50) {
     /keypress strafe_right hold
   } else {
     /keypress strafe_left hold
   }
   /delay 5
   /keypress strafe_right
   /keypress strafe_left
   /keypress forward hold

/return
|--------------------------------------------------------------------------------
|SUB: Reading from an INI File
|--------------------------------------------------------------------------------
Sub ReadINI(FileName,SectionName,ArrayType)

   /echo Attempting to Read Section "${SectionName}" Zone Information from ${FileName}...
   /delay 1s

   /if (${Ini[${FileName},${SectionName},-1,NO].Equal[NO]}) {
      /echo "${SectionName}" is not a Valid Section for FILE:${FileName}, ending macro...
      /delay 1s
      /return
   }
   /declare nValues     int local  1
   /declare nArray      int local  0
   /declare KeySet      string local  ${Ini[${FileName},${SectionName}]}

   :CounterLoop
   /if (!${KeySet.Arg[${nValues},|].Length}) {
      /varcalc nValues ${nValues}-1
      /goto :MakeArray
   }
   /varcalc nValues ${nValues}+1
   /goto :CounterLoop

   :MakeArray
   /if (!${nValues}) /return
   /if (${FileName.Equal["e3 Macro Inis\PetHunter.ini"]}&&${nValues}>0) {
      /echo Declaring Mob Array...
      /declare RV_MobArray[${nValues}]   string outer
      /declare RV_MobStats[${nValues}]   string outer
   }
   /for nArray 1 to ${nValues}
      /if (${FileName.Equal["e3 Macro Inis\PetHunter.ini"]}) {
         /varset RV_MobArray[${nArray}] ${Ini[${FileName},${SectionName},${ArrayType}${nArray}]}
         /varset RV_MobStats[${nArray}] 0
      }
   /next nArray

   /echo "${SectionName}" Zone Information Read Successfully from ${FileName}...
   /delay 1s

/return

|--------------------------------------------------------------------------------
|SUB: GM Check
|--------------------------------------------------------------------------------
Sub GMCheck

   /if (${Spawn[gm].ID}) {
      /beep
      /beep
      /beep

      /bc GM has entered the zone!
      .

      /keypress forward
      /keypress back
      /call e3_Cast ${Me.ID}} "Charm of Queen Adrianne"
      /call e3_Cast ${Me.ID}} "Advanced Charm of Queen Adrianne"
      /quit
      /endmacro
   }

/return





