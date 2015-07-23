| Hunter Macro 
| Hunter.mac 
| Author      : robdawg 
| Version     : v1.2 2004-05-13 10:06pm PST 
| Useage      : /macro Hunter 
| Description : This macro will run your character around killing any mobs in your 
|      GH_MobArray.  Then it will attempt to loot all items in your 
|      GH_LootArray.  This is definitely a work in progress and I am sure 
|      someone can think of plenty of upgrades for this. 
|------------------------------------------------------------------------------------ 

Sub startGroupHunter

   |------------------------------------------------------------ 
   |How many times should aquire target fail before delaying? 
   |------------------------------------------------------------ 
   /declare GH_FailMax          int outer  3 
   |------------------------------------------------------------ 
   |How far would you like to target a mob? 
   |------------------------------------------------------------ 
   /declare GH_MaxRadius        int outer  3000 
   |------------------------------------------------------------ 
   |How far is the combat range? 
   |------------------------------------------------------------ 
   /declare GH_Range            int outer 10 
   |------------------------------------------------------------ 
   |What is the minimum Z Value of mobs I should target? 
   |------------------------------------------------------------ 
   /declare GH_MinZRange        int outer  -1000 
   |------------------------------------------------------------ 
   |What is the maximum Z Value of mobs I should target? 
   |------------------------------------------------------------ 
   /declare GH_MaxZRange        int outer  1000 
   |------------------------------------------------------------ 
   |Should I loot all items? 
   |------------------------------------------------------------ 
   /declare GH_LootAllItems     int outer  0 
   |------------------------------------------------------------ 
   |Should I display stats? 
   |------------------------------------------------------------ 
   /declare GH_DisplayStats     int outer  1 

   |------------------------------------------------------------ 
   |Loot Array Information. 
   |------------------------------------------------------------ 
   /call ReadINI HunterMob.ini "${Zone.Name}" Mob
   /if (!${Defined[GH_MobArray]}) { 
      /echo Mob Array Creation Error, ending macro... 
      /endmacro 
   } 

   |------------------------------------------------------------ 
   |Mob Array Information. 
   |------------------------------------------------------------ 
   /call ReadINI HunterLoot.ini "${Zone.Name}" Loot 
   /if (!${Defined[GH_LootArray]}) { 
      /echo No Loot Array Created... 
   } 

   |------------------------------------------------------------ 
   |Variables that you don't need to worry about. 
   |------------------------------------------------------------ 
   /declare GH_FailCounter      int outer  0 
   /declare GH_MyTargetID       int outer  0 
   /declare GH_MyTargetName     string outer 
   /declare GH_MyTargetDead     int outer  0 
   /declare GH_InvalidTargetID  int outer  0 
   /declare GH_HasTarget        int outer  0 
   /declare GH_RandomWait       int outer  0 
   /declare GH_LootSlot         int outer  0 
   /declare GH_CheckLook        int outer  0 
   /declare GH_Fighting         int outer  0 
   /declare GH_TargetDead       int outer  0 
   /declare GH_MyXLOC           int outer  0 
   /declare GH_MyYLOC           int outer  0 
    
   /declare GH_FastRange        int outer 
   /declare GH_RangeMax         int outer 
   /declare GH_RangeMin         int outer 
   /varcalc GH_FastRange ${GH_Range}+3 
   /varcalc GH_RangeMax ${GH_Range}+1 
   /varcalc GH_RangeMin ${GH_Range}-1 

   :Start 
   /doevents 
   /call GMCheck 
   /call GroupGetTarget

   :KillAdds 
   /if (${GH_HasTarget}) /call MoveToMob 
   /if (${GH_HasTarget}) /call CombatSub 
   /if (${GH_HasTarget}) /call MoveToMob 
   /if (${GH_HasTarget} && (${Defined[GH_LootArray]} || ${GH_LootAllItems})) /call LootMob 
   /if (${GH_DisplayStats}) /call DisplayStats 
   /call ResetSub 

   /varset GH_RandomWait ${Math.Rand[5]} 
   /varcalc GH_RandomWait ${GH_RandomWait}+1 
   /echo Paranoia - Waiting ${GH_RandomWait} seconds before resuming 
   /delay ${GH_RandomWait}s 
    
   /if (${Target.ID}) { 
      /echo Looks like something is attacking us, killing it... 
      /delay 1s 
      /varset GH_HasTarget 1 
      /varset GH_Fighting 1 
      /goto :KillAdds 
   } 
    
   /goto :Start 
    
/return 

|-------------------------------------------------------------------------------- 
|SUB:Group Aquire Target
|-------------------------------------------------------------------------------- 
Sub GroupGetTarget

   /declare GH_CurrentRadius   int local 
   /declare GH_TargetSub   int local 
   :Acquire 
   /for GH_CurrentRadius 100 to ${GH_MaxRadius} step 100 
      /for GH_TargetSub 1 to ${GH_MobArray.Size} 
         /squelch /target radius ${GH_CurrentRadius} nopcnear notid ${GH_InvalidTargetID} npc "${GH_MobArray[${GH_TargetSub}]}" 
         /varset GH_MyTargetID ${Target.ID} 
         /varset GH_MyTargetDead 0 
         /if (${Target.ID}) { 
            /if (${Int[${Target.PctHPs}]}<100) { 
               /echo Mob NOT a Full Health, picking another... 
               /varset GH_InvalidTargetID ${Target.ID} 
               /call ResetSub 
               /goto :Acquire 
            } 
            /if (${Int[${Target.Z}]}<${GH_MinZRange}) { 
               /echo Mob is BELOW Min Z Range, picking another... 
               /varset GH_InvalidTargetID ${Target.ID} 
               /call ResetSub 
               /goto :Acquire 
            } 
            /if (${Int[${Target.Z}]}>${GH_MaxZRange}) { 
               /echo Mob is ABOVE Max Z Range, picking another... 
               /varset GH_InvalidTargetID ${Target.ID} 
               /call ResetSub 
               /goto :Acquire 
            } 
            /varset GH_HasTarget 1 
            /varset GH_MyTargetName ${Target.CleanName} 
            /echo Acquired ${Target.CleanName} at range ${Int[${Target.Distance}]} 
            /return 
         } 
      /next GH_TargetSub 
      /delay 2 
   /next GH_CurrentRadius 

   /if (!${Target.ID}) { 
      /delay 2s 
      /varcalc GH_FailCounter ${GH_FailCounter}+1 
      /echo Failed to Acquire Target in Range ${GH_MaxRadius} ${GH_FailCounter} Time(s) 
      /if (${GH_FailCounter}>=${GH_FailMax}) { 
         /echo Waiting for Respawns, Resetting Failure Counter... 
         /delay 60s 
         /varset GH_FailCounter 0 
      } 
      /goto :Acquire 
   } 
/return 

|-------------------------------------------------------------------------------- 
|SUB: Moving 
|-------------------------------------------------------------------------------- 
Sub MoveToMob 

   /varset GH_MyXLOC ${Int[${Me.X}]} 
   /varset GH_MyYLOC ${Int[${Me.Y}]} 
   /declare GH_DistanceTimer timer 15 
    
   /doevents 
    
   :MovementLoop 

   /if ((!${GH_Fighting})&&(!${GH_TargetDead})&&(${Target.PctHPs}<100)) { 
      /echo Mob not at full health, picking another... 
      /varset GH_InvalidTargetID ${Target.ID} 
      /varset GH_HasTarget 0 
      /call ResetSub 
      /return 
   } 

   /if (${Target.ID}) { 
      /face fast 
   } 
   /if (${Int[${Target.Distance}]}>${GH_FastRange}) { 
      /keypress forward hold 
   } 
   /if (${Int[${Target.Distance}]}<${GH_FastRange}&&${Int[${Target.Distance}]}>${GH_RangeMax}) { 
      /keypress forward 
   } 
   /if (${Int[${Target.Distance}]}<${GH_RangeMin}) { 
      /keypress back 
   } 
   /if (!${GH_DistanceTimer}) { 
      /if ((${GH_MyXLOC}==${Int[${Me.X}]})&&(${GH_MyYLOC}==${Int[${Me.Y}]})) /call HitObstacle 
      /varset GH_MyXLOC ${Int[${Me.X}]} 
      /varset GH_MyYLOC ${Int[${Me.Y}]} 
      /varset GH_DistanceTimer 15 
      /goto :Movementloop 
   } 
   /if (${Int[${Target.Distance}]}>${GH_FastRange}) /goto :MovementLoop 

/return 

|-------------------------------------------------------------------------------- 
|SUB: Combat 
|-------------------------------------------------------------------------------- 
Sub CombatSub 

   /echo Attacking Mob NOW! 
   /varset GH_Fighting 1 
   /varset GH_TargetDead 0 
    
   :CombatLoop 
   /doevents 
   /attack on 
    
   /call MoveToMob 
   /call SpecialIT 

   /if (!${Target.ID}) { 
      /attack off 
      /keypress forward 
      /keypress back 
      
      /varset GH_TargetDead 1 
      /varset GH_Fighting 0 
      /delay 1s 
      /target radius 30 corpse 
      /delay 1s 
      /if (!${Target.ID}) { 
         /call ResetSub 
         /return 
      } 
      /face fast 
   } 
   /if (!${GH_TargetDead}) { 
      /goto :CombatLoop 
   } 
    
/return 

|-------------------------------------------------------------------------------- 
|SUB: Special Combat 
|-------------------------------------------------------------------------------- 
Sub SpecialIt 

   /declare TempID    int inner  0 

   /if ((${Target.Distance}<11)&&(${Me.AbilityReady["Bash"]})) /doability "Bash" 
    
   /if ((${Int[${Me.PctHPs}]}<50)&&(${Me.Gem["Light of Nife"]})&&(${Me.SpellReady["Light of Nife"]})) { 
      /varset TempID ${Target.ID} 
      /keypress forward 
      /keypress back 
      /keypress F1 
      /echo Casting Heal Spell because of low health... 
      /cast "Light of Nife" 
      /delay 3s 
      /target id ${TempID} 
   } 
    
/return 

|-------------------------------------------------------------------------------- 
|SUB: Looting 
|-------------------------------------------------------------------------------- 
Sub LootMob 

   /declare LootSlot    int inner  0 
   /declare LootCheck   int inner  0 
   /declare LootTotal   int inner  0 
    
   /face fast 
    
   /keypress forward 
   /keypress back 
    
   /fastdrop on 
   /lootn never 
   /delay 2s 
   /loot 
   /delay 2s 
   /if (!${Corpse.Items}) { 
      /echo NO LOOT! Cheap Bastard! 
      /return 
   } 

   /varset LootTotal ${Corpse.Items} 
   /for LootSlot 1 to ${LootTotal} 
      /itemnotify loot${LootSlot} leftmouseup 
      /delay 1s 
      /if (${GH_LootAllItems}) { 
         /echo Keeping a ${Cursor.Name}... WOOT! 
         /autoinventory 
         /delay 1s 
      } else { 
         /for LootCheck 1 to ${GH_LootArray.Size} 
            /if (${Cursor.Name.Find[${GH_LootArray[${LootCheck}]}]}) { 
               /echo Keeping a ${Cursor.Name}... WOOT! 
               /varcalc GH_LootStats[${LootCheck}] ${GH_LootStats[${LootCheck}]}+1 
               /autoinventory 
               /delay 1s 
            } 
         /next LootCheck 
      } 
      /if (${Cursor.ID}) { 
         /echo Destroying a ${Cursor.Name}... 
         /destroy 
         /delay 1s 
      } 
   /next LootSlot 
    
   /notify LootWnd DoneButton leftmouseup 
   /delay 2
   
/return 

|-------------------------------------------------------------------------------- 
|SUB: Reset 
|-------------------------------------------------------------------------------- 
Sub ResetSub 

   /keypress esc 
   /keypress esc 
   /keypress esc 
   /keypress esc 
    
   /varset GH_HasTarget 0 
   /varset GH_TargetDead 0 
   /varset GH_Fighting 0 
    
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
|SUB: GM Check 
|-------------------------------------------------------------------------------- 
Sub GMCheck 

   /if (${Spawn[gm].ID}) { 
      /beep 
      /beep 
      /beep 
      
      /echo GM has entered the zone! 
      /echo FUCK HIM but ending the macro... 

      /keypress forward 
      /keypress back 

      /quit 
      /endmacro 
   } 
    
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
   /if (${FileName.Equal["HunterMob.ini"]}&&${nValues}>0) { 
      /echo Declaring Mob Array... 
      /declare GH_MobArray[${nValues}]   string outer 
      /declare GH_MobStats[${nValues}]   string outer 
   } 
   /if (${FileName.Equal["HunterLoot.ini"]}&&${nValues}>0) { 
      /echo Declaring Loot Array... 
      /declare GH_LootArray[${nValues}]  string outer 
      /declare GH_LootStats[${nValues}]  string outer 
   } 
   /for nArray 1 to ${nValues} 
      /if (${FileName.Equal["HunterMob.ini"]}) { 
         /varset GH_MobArray[${nArray}] ${Ini[${FileName},${SectionName},${ArrayType}${nArray}]} 
         /varset GH_MobStats[${nArray}] 0 
      } 
      /if (${FileName.Equal["HunterLoot.ini"]}) { 
         /varset GH_LootArray[${nArray}] ${Ini[${FileName},${SectionName},${ArrayType}${nArray}]} 
         /varset GH_LootStats[${nArray}] 0 
      } 
   /next nArray 
    
   /echo "${SectionName}" Zone Information Read Successfully from ${FileName}... 
   /delay 1s 
    
/return 

|-------------------------------------------------------------------------------- 
|SUB: Display Stats 
|-------------------------------------------------------------------------------- 
Sub DisplayStats 

   /declare nArray  int local 
    
   /if (${Defined[GH_LootArray]}) { 
      /for nArray 1 to ${GH_LootArray.Size} 
         /echo ${Int[${GH_LootStats[${nArray}]}]} ${GH_LootArray[${nArray}]}'s 
      /next nArray 
   } 
    
/return

#event doGroupHunt "<#1#> Group Hunter On#*#"
SUB event_doGroupHunt(line, ChatSender)
	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |--------Included Bot-----------|    |-----------In Zone-------------|
   /if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]}) {
      /declare doGroupHunt bool outer
      /varset doGroupHunt True
      /call startGroupHunter

   }
/return

#event stopGroupHunt "<#1#> Group Hunter Off#*#"
SUB event_stopGroupHunt(line, ChatSender)
	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |--------Included Bot-----------|    |-----------In Zone-------------|
   /if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]}) {
      /varset doGroupHunt False
      /echo doGroupHunt Stopped
   }
/return