declare @xml as xml

set @xml = 
N'<theWeelDB>
	<Axes>@axes@</Axes>
	<Objects>@objects@</Objects>
	<SortEvents>@sortEvents@</SortEvents>
</theWeelDB>'

set @xml = replace(cast(@xml as varchar(max)), '@axes@', (select * from Axis for xml auto))
set @xml = replace(cast(@xml as varchar(max)), '@objects@', (select * from Object for xml auto))
set @xml = replace(cast(@xml as varchar(max)), '@sortEvents@', (select * from SortEvent for xml auto))

select @xml