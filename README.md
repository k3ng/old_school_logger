This is an "old school" amateur radio logging program, not as old school as a pencil and paper, but about as old school as you're going to get on a computer.

This should compile on any machine that has gcc and SQLite libraries.  To compile, do:

gcc logger.c -o logger -lsqlite3

To run the logging:

./logger

You'll be initially presented with something like this:


```plaintext
K3NG's Old School Logger
2024.12.06.16.00


Current Contact Details:
  Callsign Worked: 
  Frequency: 
  Sent Report: 
  Received Report: 
  Mode: 
  Contact Date: 2024-12-08
  Contact Time: 09:32
  Note: 

>
```

One letter commands are used to update the current contact and perform operations.

```plaintext
Commands:
  h - Show thelp message
  a <ID> - erAse a contact by its ID (e.g., d 5)
  e <filename> - Export logged contacts to a CSV file (e.g., e contacts.csv)
  i <filename> - Export the database in ADIF format (e.g., i log.adif)
  l - Log a contact with the current settings
  u <ID> - Load a contact by its ID for editing (e.g., u 5)
  v - View logged contacts (options: v, v +N, v -N, v ID, v ID1-ID2)
  x - Exit the program

Field Commands:
  c - Set the callsign worked (e.g., c W3ABC)
  f - Set the frequency (e.g., f 14.250)
  s - Set the sent report (e.g., s 59)
  r - Set the received report (e.g., r 59)
  m - Set the mode (e.g., m USB, CW)
  d - Set the contact date (default: today's date)
  t - Set the contact time (default: current time)
  n - Add a note (e.g., n This is my note; l)
```

Commands can be performed one by one:

```plaintext
c k3ng
f 0755
s 589
r 339
m cw
```

Commands can also be strung together like so:

```plaintext
c k3ng f 0755 s 589 r 339 m cw
```
Once you're satisifed with the current contact fields, log the contact.

```plaintext
> l
Contact has been logged to the database.
Ready for a new contact.

Current Contact Details:
  Callsign Worked: 
  Frequency: 7055
  Sent Report: 
  Received Report: 
  Mode: CW
  Contact Date: 2024-12-08
  Contact Time: 09:42
  Note: 

>
```

You can view your contacts in the database with the v command.

```plaintext
> v

Logged Contacts:
| ID | Call Sign  | Frequency | Mode | Sent Rpt  | Recv Rpt  | Date/Time         | Notes 
| 1  | W3HCW      | 14025     | LSB  |           |           | 2024-11-24 09:52  | 1 | test 1 2 3 
| 3  | N3ABC      | 7055      | CW   | 589       | 599       | 2024-11-24 15:21  | 3            
| 5  | K3KTG      | 146025    | FM   | 59        | 57        | 2024-11-27 14:45  |              
| 6  |            |           |      |           |           | 2024-11-27 16:24  |              
| 7  |            |           |      |           |           | 2024-11-27 15:21  | test 1 2 3 | test 4 5 6 
| 8  | K3NG       | 7055      | CW   | 589       | 339       | 2024-12-08 09:32  |              

Current Contact Details:
  Callsign Worked: 
  Frequency: 7055
  Sent Report: 
  Received Report: 
  Mode: CW
  Contact Date: 2024-12-08
  Contact Time: 09:42
  Note: 

> 
```

Arguments can be used with the v command:

v 7 : Show contact #7

v 6-8 : Show contacts # 6 through 8

v +10 : Show first ten contacts

v -10 : Show last ten contacts


Logged contacts can be edited using the u command, like so:

```plaintext
K3NG's Old School Logger
2024.12.06.16.00


Current Contact Details:
  Callsign Worked: 
  Frequency: 
  Sent Report: 
  Received Report: 
  Mode: 
  Contact Date: 2024-12-08
  Contact Time: 16:09
  Note: 

> u 1
Contact ID 1 loaded into current fields.
You can now edit the current fields and use 'l' to save the changes.

Current Contact Details:
  Callsign Worked: W3HCW
  Frequency: 14025
  Sent Report: 
  Received Report: 
  Mode: LSB
  Contact Date: 2024-11-24
  Contact Time: 09:52
  Note: 1 | test 1 2 3
> s 59
Sent report set to '59'.

Current Contact Details:
  Callsign Worked: W3HCW
  Frequency: 14025
  Sent Report: 59
  Received Report: 
  Mode: LSB
  Contact Date: 2024-11-24
  Contact Time: 09:52
  Note: 1 | test 1 2 3

> l
Contact has been logged to the database.
Ready for a new contact.
```

What's Next?

I'm planning to add the following in the future:

More Fields

More Search Capabilities for the View Command

ADIF Import

Direct SQL Queries

UTC & Local Timezone Handling

MHz & kHz Frequency Handling

FCC Database Call Lookup



