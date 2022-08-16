Sub BardClassDefinedValues
    |Create a circular queue with insert and pop methods
    /declare bardSongArrayQueue[32] string outer
    /declare bardSongArrayQueueIndexFront int outer 0
    /declare bardSongArrayQueueIndexBack int outer -1
    /declare bardSongArrayQueueMax int outer 10
    /declare bardSongArrayQueueCount int outer 0
    /declare bardImportantSong string outer
    /declare bardPlayerCurrentSongType string outer
    
/RETURN
Sub BardClass_CircularQueue_Insert(InsertValue)
    /if (${bardSongArrayQueueCount}==${bardSongArrayQueueMax}) {
        /echo BardClass Queue Overflow, throwing away ${InsertValue}
        /return
    }
    /varset bardSongArrayQueueIndexBack ${Math.Calc[(${bardSongArrayQueueIndexBack} + 1) % ${bardSongArrayQueueMax}]}
    |Array indexes starting at 1 is stupid and the person should be flogged with a wet fish
    /declare arrayIndex int local ${Math.Calc[${bardSongArrayQueueIndexBack}+1]}
    |/echo Insert. Front:${bardSongArrayQueueIndexFront} back:${bardSongArrayQueueIndexBack}, index of ${arrayIndex} with value ${InsertValue}
    
    /varset bardSongArrayQueue[${arrayIndex}] ${InsertValue}
    /varcalc bardSongArrayQueueCount ${bardSongArrayQueueCount}+1


/return


Sub BardClass_CircularQueue_Pop
    /if (${bardSongArrayQueueCount}==0) {
        /echo BardClass Queue Empty Already
        /return
    }
    /declare arrayIndex int local ${Math.Calc[${bardSongArrayQueueIndexFront}+1]}
    /declare returnValue string local ${bardSongArrayQueue[${arrayIndex}]}
    /varset bardSongArrayQueueIndexFront ${Math.Calc[(${bardSongArrayQueueIndexFront} + 1) % ${bardSongArrayQueueMax}]}
    
    /varcalc bardSongArrayQueueCount ${bardSongArrayQueueCount}-1
    
    |/echo Poping ArrayIndex:${arrayIndex} with returnValue:${returnValue}
  
/return ${returnValue}

Sub BardClass_CircularQueue_Empty
    /varset bardSongArrayQueueCount 0
    /varset bardSongArrayQueueIndexFront 0
    /varset bardSongArrayQueueIndexBack -1

/return

Sub BardClass_CircularQueue_Peek
    /if (${bardSongArrayQueueCount}==0) {
        /echo BardClass Queue Empty Already
        /return
    }
    /declare returnValue string local ${bardSongArrayQueue[${Math.Calc[${bardSongArrayQueueIndexFront}+1]}]}
    
/return ${returnValue}

|0 based index
Sub BardClass_CircularQueue_PeekIndex(indexValue)
    /if (${bardSongArrayQueueCount}==0) {
        /echo BardClass Queue Empty Already
        /return
    }
   
    /declare arrayIndex int local  ${Math.Calc[(${bardSongArrayQueueIndexFront} + ${indexValue} ) % ${bardSongArrayQueueMax}]}
    /varCalc arrayIndex ${arrayIndex}+1
    /declare returnValue string local ${bardSongArrayQueue[${arrayIndex}]}
    |/echo PeekIndex Front:${bardSongArrayQueueIndexFront} back:${bardSongArrayQueueIndexBack}  of input index: ${indexValue}, array index:${arrayIndex} return value: ${returnValue}

/return ${returnValue}

Sub BardClass_CircularQueue_PrintQueue
  /if (${bardSongArrayQueueCount}==0) {
    /echo BardClass Queue Empty Already
    /return
  }
  /declare counter int local 0
  /declare itemValue string local

  |loop through the current queue,
  /while (${counter}< ${bardSongArrayQueueCount}) {
    /call BardClass_CircularQueue_PeekIndex ${counter}
    /varset itemValue ${Macro.Return}
    /echo ${counter}) ${itemValue}
    /varcalc counter ${counter}+1
  }
    
/return



Sub BardClass_CircularQueue_ToString
  /if (${bardSongArrayQueueCount}==0) {
    /echo BardClass Queue Empty Already
    /return
  }
  /declare counter int local 0
  /declare itemValue string local
  /declare returnValue string local
  |loop through the current queue,
  /while (${counter}< ${bardSongArrayQueueCount}) {
    /call BardClass_CircularQueue_PeekIndex ${counter}
    /varset itemValue ${Macro.Return}
    /varset returnValue ${returnValue} ${itemValue}
    /varcalc counter ${counter}+1
  }
    
/return ${returnValue}

|----------------------------------------------------------------------------|
| Bard Functions
|----------------------------------------------------------------------------|


sub check_BardSongs

  /if (!${songPlayer.Equal["Twist"]} || ${Me.Invis} || ${Me.Stunned} ||${Window[BigBankWnd].Open} || ${Window[MerchantWnd].Open} || ${Window[TradeWnd].Open} || ${Window[GuildBankWnd].Open}) {
      /return
  }

   |**
  Note, if the song has more than 1 second still on the time, kick out to allow other things to possibly get ready.
  this is to remove the 2 song call  before an event can take place sometimes. IE: the first 2 seconds of the 3 second song
  **|

  /call ActivateSelosKick
  
  /if (${Window[CastingWindow].Open} && ${Cast.Timing}>500) { 
    /return
  }
 
  
  /if (!${playingMelody} && ${Bool[${AssistTarget}]}) {

    /varset playingMelody TRUE

  } 

      |don't do songs when looting
  /if (${Target.CleanName.Equal[${Me}'s corpse]}) { 
      /return 
  } 
  /if (!${playingMelody} && !${fixedMelody.Equal[off]}) {
    
    /if (${Bool[${Me.Casting.ID}]} ) {
      /echo issuing stopcast2 and kicking out
      /stopcast
    } 
    /return
  } 

  
  

 

  /if (!${playingMelody}) /return
  
  /if (${bardSongArrayQueueCount}==0) {
    /return
  }
 
  
 
  /call BardSongWaitLockPumpQueue


  |######################################################
  |Figure out the next song on the list to play
  /declare songGemValue string local
  /declare nextGemValue string local
 

  
  /if (${Bool[${bardImportantSong}]} && ${bardImportantSong.Length}>0 && !${Bool[${Me.Song[${bardImportantSong}]}]}) {

    /delay 5
  }  


   /if (${Bool[${bardImportantSong}]} && ${bardImportantSong.Length}>0 && !${Bool[${Me.Song[${bardImportantSong}]}]}) {
    /bc <BardPlayer> : Rewinding to ${bardImportantSong}
    /call BardClass_CircularQueue_Pop
    /varset songGemValue ${Macro.Return}
    /call BardClass_CircularQueue_Insert "${songGemValue}"
    /declare bardSongArrayQueueCount_temp int local 0
    /while (!${songGemValue.Equal[${bardImportantSong}]}) {

      /call BardClass_CircularQueue_Pop
      /varset songGemValue ${Macro.Return}
      /call BardClass_CircularQueue_Insert "${songGemValue}"
      /varcalc bardSongArrayQueueCount_temp ${bardSongArrayQueueCount_temp} +1
      /echo bardSongArrayQueueCount_temp: ${bardSongArrayQueueCount_temp}
      /if (${bardSongArrayQueueCount_temp} > ${bardSongArrayQueueCount}) {

        /bc <BardPlayer> : Cannot rewind to ${bardImportantSong}. Error.
        /goto :normalSongSelection
      }
    }

  } else {

    :normalSongSelection
    /call BardClass_CircularQueue_Pop
    /varset songGemValue ${Macro.Return}
    /call BardClass_CircularQueue_Insert "${songGemValue}"
    

    /if (${songGemValue.Equal[Selo's Accelerating Chorus]}) {
      /if (${Me.Buff[Selo's Accelerating Chorus].Duration.TotalSeconds}>45) {
        |we already have the buff up and it has > 45 seconds left, skip it
        |/echo Skipping Selo's Accelerating Chorus as it has time left
        /call BardClass_CircularQueue_Pop
        /varset songGemValue ${Macro.Return}
        /call BardClass_CircularQueue_Insert "${songGemValue}"      
      }
    } else /if (${songGemValue.Equal[Requiem of Time]}) {
      /if (${Target.Buff[Requiem of Time].Duration.TotalSeconds}>30) {
        |If the target already has the slow on, ignore it. and skip to the next song.
        /call BardClass_CircularQueue_Pop
        /varset songGemValue ${Macro.Return}
        /call BardClass_CircularQueue_Insert "${songGemValue}"      
      }
    }

  }

  

  |End figuring out what song/item
  |##########################

 

  /declare isItem bool local False
  /declare isSpell bool local False
  /declare isAltAbility bool local False

 /if (${Bool[${Me.Book[${songGemValue}]}]}) {
      /varset isSpell True
      /varset bardPlayerCurrentSongType spell
  } else /if (${Bool[${FindItem[${songGemValue}]}]}) {
      /varset isItem True
      /varset bardPlayerCurrentSongType item
  } else /if (${Me.AltAbility[${songGemValue}]}) {
      /varset isAltAbility true
      /varset bardPlayerCurrentSongType aa
  }


  |tap the current gem if we are casting 
  /if (${Bool[${Me.Casting.ID}]} ) {
    /delay 1
    /stopcast
  }
  /if (!${playingMelody}) {
    /echo issuing stopcast and kicking out
    /stopcast
    /return
  } 
  |don't try and cast if your are stunned
  /while (${Me.Stunned}) {
    /delay 1
  }
  |########
  |Cast the Song/Item
  |check for isSpell in case we have the actual spell in our inventory
  /if (${isSpell}) {
  
    |is ths song ready? if not, kick out and loop back in
    /if (${songGemValue.Equal[Verse of Vesagran]}) { 
      /delay 1s ${Me.SpellReady[${songGemValue}]}
    }
    |a delay check, for sync 
    /if (!${Me.SpellReady[${songGemValue}]}) {
       /delay 1
    } 
    /if (!${Me.SpellReady[${songGemValue}]}) {
      /echo [${Time}]: Twist-Skip: "${songGemValue}"
      /return
    } 
    |cast the song
    /cast "${songGemValue}"
    /echo [${Time}]: Twist: "${songGemValue}"
    /delay 5 ${Window[CastingWindow].Open}
    
    /if (!${Window[CastingWindow].Open}) {
      /echo [${Time}]: fizzle happened, recasting
      /stopcast
      /delay 1
      /cast ${songGemValue}
      /delay 3
    }
  } else /if (${isItem}) {
    |item isn't ready kickout
    /if (!${Me.ItemReady[${songGemValue}]}) /return
    /echo [${Time}]: Twist-Click: "${songGemValue}"
    /useitem "${songGemValue}"
    /delay 3
   
  } else /if (${isAltAbility}) {
    /if (${Me.AltAbilityReady[${songGemValue}]}) {
       /echo [${Time}]: Twist-Alt: "${songGemValue}"
      /casting "${songGemValue}" alt
			/delay 3 !${Me.AltAbilityReady[${pendingCast}]}   
    }
  }

  |End cast of song/item
  |##############
/return


#event saveMelody "<#1#> Save Melody #2# #3#"
#event saveMelody "[MQ2] Save Melody #2# #3#"
SUB EVENT_saveMelody(line, ChatSender, melodyName, songsToSave)
/if (${Debug}) /echo |- EVENT_saveMelody ==>
	| If no ChatSender is defined, make one.
	/if (!${Defined[ChatSender]}) /declare ChatSender string local ${Me.CleanName}
	| If I'm the ChatSender.
	/if (${ChatSender.Equal[${Me.CleanName}]}) {
		/declare i int local
		| Check if the melody already exists, and should not be overwritten.
		/if (${Ini[${Character_Ini},${melodyName} Melody].Length}) {	
			/if (${Debug}) /echo |- EVENT_saveMelody -| Overwriting [${melodyName}] melody...
			/for i 1 to 8 + ${Me.SPA[326]}
				/if (${Ini[${Character_Ini},${melodyName} Melody,Song].Length}) /call WriteToIni "${Character_Ini},${melodyName} Melody,Song#${i}" *PLACEHOLDER 1
			/next i
		}
		/echo Saving [${melodyName}] melody...
		/varset i 1
		:check_nextSong

		| If the gem number is not recognized
		/if (!${Int[${songsToSave.Arg[${i}, ]}]} || ${Int[${songsToSave.Arg[${i}, ]}]} < 1 || ${Int[${songsToSave.Arg[${i}, ]}]} > 9) {
			/if (${songsToSave.Arg[${i}, ].NotEqual[-Overwrite]}) {
				/echo [${songsToSave.Arg[${i}, ]}] is not a valid gem number.  Please use the following syntax when saving melodies, "Save Melody [NAME] [# # # # #] (-Overwrite)".  Ex.  /echo Save Melody Raid 1 2 3 4 5 6 7 -Overwrite
			}
		} else {
			/echo Song#${i}=${Me.Gem[${songsToSave.Arg[${i}, ]}]}/gem|${songsToSave.Arg[${i}, ]}
			/call WriteToIni "${Character_Ini},${melodyName} Melody,Song" "${Me.Gem[${songsToSave.Arg[${i}, ]}]}/gem|${songsToSave.Arg[${i}, ]}" ${If[${Ini[${Character_Ini},${melodyName} Melody,Song].Length},1,0]}
		}
		/varcalc i ${i} + 1
		/if (${songsToSave.Arg[${i}, ].Length}) /goto :check_nextSong
	}
/if (${Debug}) /echo <== EVENT_saveMelody -|
/return
|------------------------------------------------------------------------|

#EVENT startMelody "<#1#> Melody #2#"
#EVENT startMelody "[#1#] Melody #2#"
#EVENT startMelody "#1# tells you, 'Melody #2#'"

SUB EVENT_startMelody(line, ChatSender, melodyName)
  
  /if (!${Me.Class.ShortName.Equal[BRD]}) /return

  /if (${Debug}) /echo |- EVENT_startMelody ==>

  |getting the melody name via c_eventArgs? odd 
  /if (!${checkEventArgs[${ChatSender},${line},UZR,${melodyName}]}) /return
 

  /varset melodyName ${c_eventArgData}
  /declare memGem int local
  /declare songList string local
  
  |check for commands first
  /if (${Select[${melodyName},Stop,End,Pause,Resume]}) {
    | If Stop, end, or pause were used, stop the melody.
    /if (${Select[${melodyName},Stop,End]}) {
      |empty out the queue
      /call BardClass_CircularQueue_Empty
      /echo setting playingMelody FALSE
      /varset playingMelody FALSE
      /varset fixedMelody off

      /if (${songPlayer.Equal["MQ2Twist"]}) {
          /varset songSet
          /docommand ${ChatToggle} Ending melody.
          /twist stop
          |let gems reset
          /delay 3
      }
      /stopcast
     
    } else /if (${Select[${melodyName},Pause]}) { 
      
      /call pauseTwist

    } else /if (${Select[${melodyName},Resume]}) { 

      /call unpauseTwist

    } else {

      /docommand ${ChatToggle} EVENT_startMelody [${melodyName}] is not a known melody.
    }
    |###### RESUME END
  } else {
    |now lets check for melodies sent to us
    /call BardClass_CircularQueue_Empty
    
    /call IniToArrayV "${Character_Ini},${melodyName} Melody,Song#" melodyArray
    /if (!${Defined[melodyArray]}) {
        /varset playingMelody FALSE
        /docommand ${ChatToggle} [${melodyName}] is not a known melody.
    } else {
          /varset currentMelody ${melodyName}
          |/varset fixedMelody ${melodyName}
          /declare spellName string local
          /declare i int local
          | For 1 to 7 (maximum melody size to include selos, 6 songs played actively with some droppage)
          /for i 1 to ${melodyArray.Size}
            |/echo ${i} ${melodyArray[${i}]}
            /if (${melodyArray[${i}].Length} && ${melodyArray[${i}].NotEqual[PLACEHOLDER]}) {
              /varset spellName ${melodyArray[${i}].Arg[1,/]}
              | Check that the song is in my book or the item is in my inventory
              /if (${Me.Book[${spellName}]}) {
                | If the song is not memmed, use mq2Cast to mem
                /if (!${Me.Gem[${spellName}]}) {
                  /call argueString gem| "${melodyArray[${i}]}"
                  /if (${Bool[${c_argueString}]}) {
                    /varset memGem ${c_argueString}
                  } else {
                    /varset memGem ${DefaultGem}
                  }
                  /memorize "${spellName}" "${memGem}"
                  /if (${Cast.Status.Find[M]}) /delay 3s !${Cast.Status.Find[M]}
                } 
                
                /if (${melodyArray[${i}].Find[/ImportantSong]}) {
                    /echo Setting important song:${spellName}
				            /varset bardImportantSong ${spellName}
			          }
                
                | Update songList with the corresponding gem#
                /varset songList ${songList} ${Me.Gem[${spellName}]}
                /if (${songPlayer.Equal["Twist"]}) {
                    /call BardClass_CircularQueue_Insert  "${spellName}"
                } else {
                    /call BardClass_CircularQueue_Insert  "${Me.Gem[${spellName}]}"
                }
                
              } else /if (${Bool[${FindItem[=${spellName}]}]}) {
                /call argueString gem| "${melodyArray[${i}]}"
                /echo Found item for twist ${spellName}, will use item slot ${c_argueString}
                /varset songList ${songList} ${c_argueString}
                /if (${songPlayer.Equal["Twist"]}) {
                     /call BardClass_CircularQueue_Insert  "${spellName}"
                } else {
                    /call BardClass_CircularQueue_Insert ${c_argueString}
                }
              } else /if (${songPlayer.Equal["Twist"]} && ${Me.AltAbility[${spellName}]} ) {
                   /call BardClass_CircularQueue_Insert  "${spellName}"
              } else {
                /echo I don't have [${spellName}], playing the melody without it.
              } 
            }
          /next i
        
          | If I have a songList, start singing the melody
          /if (${Bool[${songList}]}) {
            | Twist the newly built songList.
            /varset playingMelody TRUE
            /varset returnTwist FALSE
            /varset songSet ${songList}
            |/echo singing Songset: ${songList}
            /docommand ${ChatToggle} Playing New Melody: [${melodyName}].
            /if (${songPlayer.Equal["MQ2Twist"]}) {
                /twist ${songSet}
            }
           
          }

          /call BardClass_CircularQueue_PrintQueue
        }
    }

 
  :endStartMelody
  
  |for debug purposes
  |/call BardClass_CircularQueue_PrintQueue

/if (${Debug}) /echo <== EVENT_startMelody -|

/return
|--------------------------------------------------------------|
SUB pauseTwist
/if (${Debug} || ${Debug_Casting}) /echo |- pauseTwist ==>
	
  /varset playingMelody FALSE
 
   /if (!${songPlayer.Equal["MQ2Twist"]}) {
      /return
   }
  
    /varset songSet
    /docommand ${ChatToggle} Pausing: ${fixedMelody}.
    /twist stop
    |let gems reset
    /delay 3
    
    /declare counter int local 0
    /declare songGemValue string local

    |loop through the current queue, poping off at the front, and reinserting back at the end
    /while (${counter}< ${bardSongArrayQueueCount}) {
      /call BardClass_CircularQueue_Peek
      /varset songGemValue ${Macro.Return}
      |found the variable that equals the last gem played
      /if (!${songGemValue.Equal[${previousSpellGemThatWasCast}]}) {

            /call BardClass_CircularQueue_Pop
            /varset songGemValue ${Macro.Return}
            /call BardClass_CircularQueue_Insert "${songGemValue}"

      } else {

        |it equals so kick out we are done
        /break
      } 
      /varcalc counter ${counter}+1
    }      
  
  
/if (${Debug} || ${Debug_Casting}) /echo <== pauseTwist -|
/return
|--------------------------------------------------------------|
|--------------------------------------------------------------|

SUB unpauseTwist
  
 
  /if (!${songPlayer.Equal["MQ2Twist"]}) {
      /varset playingMelody TRUE
      /return
   }
  
	/delay 3s !${Window[CastingWindow].Open}
	
  /if (!${Bool[${playingMelody}]}) {
     /declare songList string local
    |###### RESUME
    |reread the playlist and start up the twist again
    /if (${bardSongArrayQueueCount}>0) {
      
      /call BardClass_CircularQueue_ToString
      /varset songList ${Macro.Return}
      
      | Twist the newly built songList.
      /varset playingMelody TRUE
      /varset returnTwist FALSE
      /varset songSet ${songList}
      |/echo singing Songset: ${songList}
      /docommand ${ChatToggle} Unpause [${fixedMelody}].
      /if (${songPlayer.NotEqual["Dynamo"]}) {
       
        /twist ${songSet}
      } 
      /delay 1s ${Window[CastingWindow].Open}
    
    } else {

      /docommand ${ChatToggle} No melody to resume.
    }
  } else {
    /docommand ${ChatToggle} unpauseTwist: [${fixedMelody}] is not a known melody.
  }
  |###### RESUME END

/return
|--------------------------------------------------------------|
#event toggleMez "<#1#> Mez #2#"
#event toggleMez "#1# tells you, 'Mez #2#'"
sub event_toggleMez(line, ChatSender, mezToggle)
  /if (${Me.Class.ShortName.Equal[ENC]} || ${Me.Class.ShortName.Equal[BRD]}) {
    /if (${Debug} || ${Debug_Ench}) /echo |- remove_RuneTarget ==>
      /if (${mezToggle.Equal[on]}) {
        /varset autoMez TRUE
        /varset fixedMelody ${currentMelody}
        /docommand ${ChatToggle} << Mezzing Enabled >>
      } else {
        /varset autoMez FALSE
        /varset fixedMelody off
        /docommand ${ChatToggle} << Mezzing Disabled >>
    }
    /if (${Debug}) /echo <== remove_RuneTarget -|
  }
/return
|--------------------------------------------------------------|
Sub check_bard_charm
/if (${Debug} || ${Debug_Ench}) /echo |- check_Charm ==>
/if (${Defined[charmArray2D]}) {
  /declare s int local
  /declare charmDuration int local
  /if (${doCharm} && ${Bool[${charmTarget}]}) {
    /if (${SpawnCount[npc id ${charmTarget}]}==0 && ${charmTarget}!=${Me.Pet.ID}) {
      /docommand ${ChatToggle} Disabling Charm: ${charmTarget} is no longer a valid npc
      /varset doCharm FALSE
      /varset charmTarget 0
      /return
    }
    /if (${Me.Pet.Race.Name.Equal[Invisible Man]}) {
      /pet get lost
      /delay 1s !${Me.Pet.ID}
    }
    /if (!${Bool[${Me.Pet.ID}]}) {
      /bc [+r+] << ${Spawn[id ${charmTarget}].CleanName} is not Charmed! >> [+r+]
      /for s 1 to ${charmArray2D.Size[1]}
      /call check_Ready "charmArray2D" ${s}
      /if (${c_Ready}) {
        /if (${check_Mana["charmArray2D",${s}]}) {
          /if (${check_Distance[${charmTarget} ${charmArray2D[${s},${iMyRange}]}]}) {
            /docommand ${ChatToggle} << Charming ${charmTarget} ${Spawn[id ${charmTarget}].CleanName} with ${charmArray2D[${s},${iCastName}]} >>
            /call e3_Cast ${charmTarget} "charmArray2D" "${s}"
            /if (${Bool[${Me.Pet.ID}]}) {
              /bc [+g+] >> Charmed ${Spawn[id ${charmTarget}].CleanName} <<
              /timed 5 /pet back off
              /timed 10 /pet follow
            } else {
              /if (${castReturn.Equal[CAST_IMMUNE]}) {
                /varset doCharm FALSE
                /varset charmTarget 0
                /docommand ${ChatToggle} << ${Spawn[id ${charmTarget}].CleanName} is immune, turning off charm >>
              }
            }
          }
        }
      }
      /if (!${Bool[${Me.Pet.ID}]}) /next s
    } else {
      |accounce time left on charm
      /for s 1 to ${charmArray2D.Size[1]}
      /if (${Bool[${Me.PetBuff[${charmArray2D[${s},${iSpellName}]}]}]}) {
        /varset charmDuration ${Pet.BuffDuration[${charmArray2D[${s},${iSpellName}]}].TotalSeconds}
        |/echo charmduration ${charmDuration} timer ${charmBuffTimer}
        /if (${charmDuration} > 60) {
          /if (!${Bool[${charmBuffTimer}]}) {
            /bc [+g+] My charm fades in ${charmDuration} sec
            /varset charmBuffTimer 18s
          }
        } else /if (${charmDuration} <= 60 && ${charmDuration} > 9) {
          /if (!${Bool[${charmBuffTimer}]}) {
            /bc [+y+] My charm fades in ${charmDuration} sec
            /varset charmBuffTimer 6s
          }
        } else /if (${charmDuration} <= 9) {
          /if (!${Bool[${charmBuffTimer}]}) {
            /bc [+r+] My charm fades in ${charmDuration} sec
            /varset charmBuffTimer 1s
          }
        }
      }
      /next s
    }
  }
}
/if (${Debug} || ${Debug_Ench}) /echo <== check_Charm -|
/return
|--------------------------------------------------------------|
| Mez mobs in ae radius that are not assist target
sub check_bard_mez
  /if (${Debug} || ${Debug_Ench}) /echo |- check_Mez ==>
  /if (${autoMez}) {
    /declare i int local
    /declare s int local
    /declare mezTarg int local
    /if (!${SpawnCount[playerstate 4 npc targetable radius ${AE_Radius}]}) {
      /varset returnTwist TRUE
      /return
    }
    /for i 1 to ${SpawnCount[playerstate 4 npc targetable radius ${AE_Radius}]}
      /varset mezTarg ${NearestSpawn[${i},playerstate 4 npc targetable radius ${AE_Radius}].ID}
      /if (${Spawn[id ${mezTarget}].CleanName.Find[' pet]}) /next i
      /call TrueTarget ${mezTarg}
      /delay 1s ${Target.BuffsPopulated}
      /if (${Select[${AssistTarget},${mezTarg}]}==0 && (!${Bool[${Target.Mezzed}]} || ${Target.Mezzed.Duration.TotalSeconds} < 7)) {
        /for s 1 to ${mezArray2D.Size[1]}
          /if (${check_Distance[${mezTarg},${mezArray2D[${s},${iMyRange}]}]}) {
            /if (${mezArray2D[${s},${iCastType}].Equal[Spell]} && !${Bool[${Me.Gem[${mezArray2D[${s},${iCastName}]}]}]}) /call memorize_spell "${mezArray2D[${s},${iCastName}]}" ${mezArray2D[${s},${iSpellGem}]}
              /docommand ${ChatToggle} << Mezzing ${mezTarg} with ${mezArray2D[${s},${iCastName}]} >>
              /if (${Twist.Twisting}) /call pauseTwist
              /attack off
              |:retryMez
              /twist once ${mezArray2D[${s},${iSpellGem}]}
              /delay 3s 
              /delay 1s ${Target.BuffsPopulated}

              /if (${returnTwist}) /call unpauseTwist
            }
        /if (!${ActionTaken}) /next s
      }
    /if (!${ActionTaken}) /next i
  }
  /if (${Debug}) /echo <== check_Mez -|
/return
|--------------------------------------------------------------|

| Change melodies based on MelodyIf statements
SUB melodyIfs

  /if (!${playingMelody}) /return
  | when fixedMelody is stop, stop all melodies and don't let them start
  | when fixedMelody is off/null, start up MelodyIf again
  /if (!${Bool[${fixedMelody.Equal[off]}]} && !${currentMelody.Equal[${fixedMelody}]}) {
    
    |/echo if starting up melody ${fixedMelody}

    /call EVENT_startMelody "<${Me}> melody ${fixedMelody}" "${Me}" "${fixedMelody}"
    /return
  } else /if (${currentMelody.Equal[${fixedMelody}]}) {
    |/echo  already set to one of the fixed melodies
    /return
  }

  /if (!${Defined[melodyIfs]}) /return
  /declare i int local
  /declare newIfMelody string local 
  /for i 1 to ${melodyIfs.Size[1]}
    /varset newIfMelody ${melodyIfs[${i},1]}
    /if (!${currentMelody.Equal[${newIfMelody}]} && (${melodyIfs[${i},2]})) {
       
      |/echo if starting up melody:${newIfMelody}

      /call EVENT_startMelody "<${Me}> melody ${newIfMelody}" "${Me}" "${newIfMelody}"
    }
  /next i
 
/RETURN

|--------------------------------------------------------------|

#event setFixedMelody "<#1#> Set Melody #2#"
SUB event_setFixedMelody(line, ChatSender, melodyName)
  | Set to a fixed melody
  /varset fixedMelody ${melodyName}
/RETURN

|--------------------------------------------------------------|

| Automatically cast Selo's Sonata whenever the buff is missing on the Bard
SUB autoSonata
  /if (${autoSonataOn} && (!${Me.Buff[Selo's Sonata].ID} || ${Me.Buff[Selo's Sonata].Duration.TotalSeconds} < 10) && !${Me.Invis} && ${Me.AltAbilityReady[Selo's Sonata]}) {
    /alt act 3704
  }
/RETURN

|--------------------------------------------------------------|
Sub BRD_Setup
/if (${Debug}) /echo |- BRD_Setup ==>
/declare autoMez bool outer FALSE
	/call iniToVarV "${Character_Ini},Bard,Auto-Melody" autoMelody string outer
  /call iniToVarV "${Character_Ini},Bard,AutoMez (On/Off)" autoMez bool outer
  /call IniToArrayV "${Character_Ini},Bard,Mez#"	  mezArray
  /call IniToArrayV "${Character_Ini},Bard,Charm#"	charmArray
  /call IniToArrayV "${Character_Ini},Bard,MelodyIf#"	melodyIfArray
  /call iniToVarV "${Character_Ini},Bard,Auto-Sonata (On/Off)" autoSonataOn bool outer
  /call iniToVarV "${Character_Ini},Bard,SongPlayer" songPlayer string outer
  /call iniToVarV "${Character_Ini},Bard,DynamoPrecastTime" dynamoPrecastTime int outer
  /declare returnTwist bool outer False
  /if (!${Defined[songPlayer]}) {
    /echo Setting default SongPlayer to Twist
    /declare songPlayer string outer Twist
  }

  /if (!${Defined[dynamoPrecastTime]}) {
    /declare dynamoPrecastTime int outer 4000
  }

  
  /declare playingMelody	bool outer TRUE
  /declare songSet string outer
  /declare resumeTwistDelay timer outer
  /declare songCastTimer timer outer
  /declare currentlyCastingSong string outer none
  /declare currentlyCastingSongNhtTimerName string outer none
  /declare previousCastSong string outer none

  /if (${Defined[mezArray]}) {
    /call BuildSpellArray "mezArray"	"mezArray2D"
  } else {
    /if (${Defined[autoMez]}) /varset autoMez FALSE
  }

  /declare charmTarget int outer
  /if (${Defined[charmArray]}) {
    /call BuildSpellArray "charmArray"	"charmArray2D"
    /declare doCharm bool outer TRUE
    /declare charmBuffTimer timer outer
  } else {
    /declare doCharm bool outer FALSE
  }

  | Setup MelodyIf configuration
  /declare currentMelody string outer ${autoMelody}
  /declare fixedMelody string outer off
  /if (${Defined[melodyIfArray]}) {
    /declare melodyIfs[${melodyIfArray.Size},2] string outer 0
    /declare i int local
    /for i 1 to ${melodyIfArray.Size[1]}
      /varset melodyIfs[${i},1] ${melodyIfArray[${i}].Arg[1,/]}
      /call argueString Ifs| "${melodyIfArray[${i}]}"
      /varset melodyIfs[${i},2] ${Ini[${Character_Ini},Ifs,${c_argueString},NULL,noparse]}
    /next i
  }

  | Clear songs that were playing from previous macro
  /stopsong
  /twist end
/if (${Debug}) /echo <== BRD_Setup -|
/return
|--------------------------------------------------------------|
Sub BRD_Background_Events
  /if (${returnTwist} && !${Bool[${Me.Casting}]} && !${Twist.Twisting} && !${resumeTwistDelay}) /call unpauseTwist
	/doevents saveMelody
	/doevents startMelody
  /doevents charmTarget
  /doevents toggleMez
  /doevents charmOn
  /doevents charmOff
  /doevents setFixedMelody
  /call autoSonata
  /call check_Dynamo
  /call check_BardSongs
  /call melodyIfs
/return
|--------------------------------------------------------------|
SUB BRD_MacroSettings
	| Adv Settings
	/call WriteToIni "${advSettings_Ini},BRD Functions,BRD Function#1" "check_Burns" 1
	/call WriteToIni "${advSettings_Ini},BRD Functions,BRD Function#2" "check_Buffs" 1
	/call WriteToIni "${advSettings_Ini},BRD Functions,BRD Function#3" "check_bard_mez" 1
/RETURN
|--------------------------------------------------------------|
SUB BRD_CharacterSettings
	/call WriteToIni "${Character_Ini},Bard,Auto-Melody"
  /call WriteToIni "${Character_Ini},Bard,AutoMez (On/Off)" Off
  /call WriteToIni "${Character_Ini},Bard,Mez"
  /call WriteToIni "${Character_Ini},Bard,Charm"
  /call WriteToIni "${Character_Ini},Bard,Auto-Sonata (On/Off)" Off
  /call WriteToIni "${Character_Ini},Bard,MelodyIf"
  /call WriteToIni "${Character_Ini},Bard,SongPlayer" Twist
/RETURN
|--------------------------------------------------------------|
Sub BRD_Aliases
	/squelch /alias /SaveMelody /echo Save Melody
	/squelch /alias /PlayMelody /bc Melody
  /squelch /alias /mezon /bc Mez on
  /squelch /alias /mezoff /bc Mez off
/return

SUB dynamoTwist  
  /if (${songPlayer.NotEqual["Dynamo"]} || ${Me.Invis}) /return

  | Read auto-melody for starting melody
  | If the target is self, read through the list and cast the first one that's not on
  | If the spell type is detrimental, check if I have a target, and if I do go cast that one on the target if it's not on yet
  | may need a "rotate" if you want to skip when it doesn't 
  /if (!${Bool[${Me.Casting}]} && ${Defined[melodyArray]}) {
    /declare spellName string local
    /declare castName string local
    /declare i int local 0
    /declare songIf string local
    /declare ifsPassed bool local TRUE
    /declare tid int local 0
    /declare castTime int local 33

    /for i 1 to ${melodyArray.Size}
      /varset spellName ${melodyArray[${i}].Arg[1,/]}
      /varset castName ${spellName}
      /varset ifsPassed TRUE
      /varset castTime 33
      |/echo ${melodyArray[${i}]}
      /if (${Bool[${melodyArray[${i}].Find[Ifs]}]}) {
        /call argueString Ifs| "${melodyArray[${i}]}"
        /varset songIf ${Ini[${Character_Ini},Ifs,${c_argueString},NULL,noparse]}
        |/echo checking ${spellName} for ${c_argueString}
        /if (!${songIf}) {
          |/echo skipping ${spellName} for ${c_argueString}
          /varset ifsPassed FALSE
        }
      }

      /if (${ifsPassed}) {
        | Handle items
        /if (${FindItemCount[=${spellName}]}) {
          /varset castTime ${Math.Calc[${FindItem[=${spellName}].CastTime}/100 + 3]}
          /varset spellName ${FindItem[=${spellName}].Spell} 
        }

        /if (${Spell[${spellName}].SpellType.Equal[Beneficial]}) {
          /varset tid ${Me.ID}
        } else {
          /varset tid ${Target.ID}
        }

        /if (!${Defined[nhtTimerName]}) {
          /declare nhtTimerName string local song-nht${tid}-${Spell[${spellName}].ID}
        } else {
          /varset nhtTimerName song-nht${tid}-${Spell[${spellName}].ID}
        }

        |/echo nhtTimerName = ${nhtTimerName}

        /if (${Defined[${nhtTimerName}]}) {
          |/echo ${nhtTimerName} Timer defined
          /if (${${nhtTimerName}} > 0) {
            |/echo skipping ${spellName} for timer ${nhtTimerName}, current time: ${${nhtTimerName}}
            /if (${i} < ${melodyArray.Size}) {
              /next i
            } else {
              /return
            }
          }
        }

        /if (${previousCastSong.Equal[${spellName}]}) {
          |/echo Sang ${spellName} previously, skipping to next song
          |/echo previousCastSong = ${previousCastSong}
          /call delaySongCast "${previousCastSong}" "${nhtTimerName}"
          /varset previousCastSong none
          |/echo ${previousCastSong}
          /if (${i} < ${melodyArray.Size}) {
            |/echo going to next i
            /next i
          } else {
            |/echo no more songs, returning
            /return
          }
        }

        |/echo ${Spell[${spellName}].SpellType.Equal[Beneficial]} && (!${Bool[${Me.Song[${spellName}]}]} || ${Me.Song[${spellName}].Duration} < ${dynamoPrecastTime}) && (${castName.NotEqual[${spellName}]} || ${Me.SpellReady[${spellName}]})
        /if (${Spell[${spellName}].SpellType.Equal[Beneficial]} && (!${Bool[${Me.Song[${spellName}]}]} || ${Me.Song[${spellName}].Duration} < ${dynamoPrecastTime}) && (${castName.NotEqual[${spellName}]} || ${Me.SpellReady[${spellName}]})) {
          |/echo Singing ${castName}
          /if (${castName.NotEqual[${spellName}]}) {
            /itemnotify "${castName}" rightmouseup
          } else {
            /cast "${castName}"
          }

          /varset previousCastSong ${spellName}
          |/echo Casting=${Me.Casting.Name}
          /if (${Me.Casting.Name.Equal[${spellName}]} || ${castTime} == 0) {
            /varset currentlyCastingSong ${spellName}
            /varset currentlyCastingSongNhtTimerName ${nhtTimerName}
            /varset songCastTimer ${castTime}

            |/echo ${spellName} songCastTimer = ${songCastTimer}
          }
    
          /return
        } else /if (${Spell[${spellName}].SpellType.Equal[Detrimental]} && ${Bool[${Target.Type.Equal[NPC]}]} && (!${Bool[${Target.Buff[${spellName}]}]} || ${Target.BuffDuration[${spellName}]} < 3300) && (${castName.NotEqual[${spellName}]} || ${Me.SpellReady[${spellName}]}) && ${Target.LineOfSight}) {
          |/echo Singing ${castName}

          /if (${castName.NotEqual[${spellName}]}) {
            /itemnotify "${castName}" rightmouseup
            /delay 5
          } else {
            /cast "${castName}"
          }

          |/echo Casting=${Me.Casting.Name}

          /varset previousCastSong ${spellName}
          /if (${Me.Casting.Name.Equal[${spellName}]} || ${castTime} == 0) {
            /varset currentlyCastingSong ${spellName}
            /varset currentlyCastingSongNhtTimerName ${nhtTimerName}
            /varset songCastTimer ${castTime}

            |/echo ${spellName} songCastTimer = ${songCastTimer}
          }

          /return
        }
      }      
    /if (${i} < ${melodyArray.Size}) /next i
  }
/return

SUB check_Dynamo
  /if (${songPlayer.NotEqual["Dynamo"]} || ${Me.Invis} || !${Defined[melodyArray]}) /return

  | If currently casting and haven't run out of time to cast, skip check
  /if (${Bool[${Me.Casting}]} && ${songCastTimer} > 0) /return

  /if (${Bool[${Me.Casting}]}) {
    |/echo stopping ${Me.Casting}, songCastTimer = ${songCastTimer}
  }

  /stopsong
  
  |/echo currentlyCastingSongNhtTimerName=${currentlyCastingSongNhtTimerName}

  | Check for setting delay timer
  /if (${Spell[${currentlyCastingSong}].SpellType.Equal[Beneficial]}) {
    | If the Beneficial spell does not stick, delay spellcast time and double each time
    /if (!${Bool[${Me.Song[${currentlyCastingSong}]}]}) {
      /call delaySongCast "${currentlyCastingSong}" "${currentlyCastingSongNhtTimerName}"
    } else /if (${Defined[${currentlyCastingSongNhtTimerName}]}) {
      |/echo resetting timer for ${currentlyCastingSong}
      | Song successfually cast, reset the timer to 0
      /varset ${currentlyCastingSongNhtTimerName} 0
    }
  } else {
    | If the Detrimental spell does not stick, delay spellcast time and double each time
    /if (!${Bool[${Target.Buff[${currentlyCastingSong}]}]}) {
      /call delaySongCast "${currentlyCastingSong}" "${currentlyCastingSongNhtTimerName}"
    } else /if (${Defined[${currentlyCastingSongNhtTimerName}]}) {
      | Song successfually cast, reset the timer to 0
      /varset ${currentlyCastingSongNhtTimerName} 0
    }
  }

  /varset currentlyCastingSong none
  /varset currentlyCastingSongNhtTimerName none

  /call dynamoTwist  
/RETURN

SUB delaySongCast(songName, myNhtTimerName)
  /if (${songName.Equal[none]}) {
    /return
  }

  | spell did not take hold, create no hold timer
  /if (!${Defined[${myNhtTimerName}]}) {
    | create a new no hold timer
    |/echo Creating new no hold timer ${myNhtTimerName}
    /declare ${myNhtTimerName} timer outer 30
  } else {
    | Increase the no hold timer from the previous time, up to 1 minute
    /if (${${myNhtTimerName}.OriginalValue} < 600) /varset ${myNhtTimerName} ${Math.Calc[${${myNhtTimerName}.OriginalValue} + 60]}
  }
  |/echo ${songName} did not take hold, waiting ${myNhtTimerName} = ${${myNhtTimerName}.OriginalValue} before trying again
/RETURN

sub ActivateSelosKick

  /if (${Target.ID} && ${Target.ID} == ${AssistTarget} && ${Me.AltAbilityReady[Selo's Kick]}  && ${Spawn[${Target}].Distance} < 69 ) {

    /alt activate 8205
    /delay 5 !${Me.AltAbilityReady[Selo's Kick]}
   
  }

/RETURN
sub BardSongWaitLockPumpQueue
  /if (!${playingMelody} ) {
    /echo issuing stopcast and kicking out
    /stopcast
    /return
  } 
|Block in case we are currently casting a previous song
  /while (${Window[CastingWindow].Open}) {
    
    /doevents nowCast

    /if (${nowSpellArrayQueueCount}>0) {
        |check to see if we have spell(s) ready in nowCast that may have interrupted the above call
        /call castNowSpells

    }
    /doevents startMelody
    /if (!${playingMelody}) {
      /echo issuing stopcast and kicking out
      /stopcast
      /return
    }
    /call ActivateSelosKick

    /doevents Follow
    /if (!${Window[CastingWindow].Open}) {
      /break
    }
    /doevents Stop
    /if (!${Window[CastingWindow].Open}) {
      /break
    }
    /doevents MoveHere
    /if (!${Window[CastingWindow].Open}) {
      /break
    }
    /doevents BackOff
    /if (!${Window[CastingWindow].Open}) {
      /break
    }
    /doevents clickit
    /if (!${Window[CastingWindow].Open}) {
      /break
    }
    /doevents bark
    /if (!${Window[CastingWindow].Open}) {
      /break
    }
    /call alerts_Background_Events
    /if (!${Window[CastingWindow].Open}) {
      /break
    }
    /doevents Assist
    /if (!${Window[CastingWindow].Open}) {
      /break
    }
    /doevents BackOff
    /if (!${Window[CastingWindow].Open}) {
      /break
    }
    /if (${Auto_Loot}) {
      /call check_Loot "on"
    
    }
    /call check_Anchor
    /if (!${Window[CastingWindow].Open}) {
      /break
    }
    /delay 1
  }

/RETURN