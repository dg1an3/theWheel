select * from Object
	INNER JOIN FREETEXTTABLE(Object, Description, 'issue OR fix OR correct OR debug OR assert') As CtnTable 
		ON CtnTable.[KEY] = Object.ObjectId
ORDER BY RANK

select * from Object
	INNER JOIN FREETEXTTABLE(Object, Description, 'memory') As CtnTable 
		ON CtnTable.[KEY] = Object.ObjectId
ORDER BY RANK

select * from Object
	INNER JOIN FREETEXTTABLE(Object, Description, 'calc OR calculation OR algorithm') As CtnTable 
		ON CtnTable.[KEY] = Object.ObjectId
ORDER BY RANK

select * from Object
	INNER JOIN FREETEXTTABLE(Object, Description, 'region OR roi OR structure OR "binary volume"') As CtnTable 
		ON CtnTable.[KEY] = Object.ObjectId
ORDER BY RANK

select * from Object
	INNER JOIN FREETEXTTABLE(Object, Description, 'space OR resolution OR grid OR resample OR pyramid OR transformation') As CtnTable 
		ON CtnTable.[KEY] = Object.ObjectId
ORDER BY RANK

select * from Object
	INNER JOIN FREETEXTTABLE(Object, Description, 'image OR volume') As CtnTable 
		ON CtnTable.[KEY] = Object.ObjectId
ORDER BY RANK

select * from Object
	INNER JOIN FREETEXTTABLE(Object, Description, 'class OR object OR data OR refactor OR method') As CtnTable 
		ON CtnTable.[KEY] = Object.ObjectId
ORDER BY RANK

select * from Object
	INNER JOIN FREETEXTTABLE(Object, Description, 'dicom OR import') As CtnTable 
		ON CtnTable.[KEY] = Object.ObjectId
ORDER BY RANK



select * from Object
	INNER JOIN FREETEXTTABLE(Object, Description, 'beam OR plan OR dose') As CtnTable 
		ON CtnTable.[KEY] = Object.ObjectId
ORDER BY RANK



select * from Object 	
	INNER JOIN FREETEXTTABLE(Object, Description, 'entropy OR damping OR variance OR KL OR histogram OR DVH') As CtnTable 
		ON CtnTable.[KEY] = Object.ObjectId
ORDER BY RANK

