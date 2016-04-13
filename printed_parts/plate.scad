
//Thickness of entire plate
plateThickness=3;
//Unit square length, from Cherry MX data sheet
lkey=19.05;
//Hole size, from Cherry MX data sheet
holesize=14;
//length, in units, of board
width=4;
//Height, in units, of board
height=1;
//Radius of mounting holes
mountingholeradius=2;
//Width around the mounting holes
mountingtabwidth=2+mountingholeradius;
//Tab position ratio
taboffsetratio = 1/4;
//height of switch clasp cutouts
cutoutheight = 3;
//width of switch clasp cutouts
cutoutwidth = 1;

//calculated vars

holediff=lkey-holesize;
w=width*lkey;
h=height*lkey;


//my custom keyboard layout layer
myKeyboard = [
//start column 0
	[[0,0],1.5],
//start column 1
	[[1.5,0],1],
//start column 2
	[[2.5,0],1],
];

//poker keyboard layout layer
pokerkeyboard = [
//start ROW 0
[[0,0],1],
[[1,0],1],
[[2,0],1],
[[3,0],1],
];

module extendedMountingHole(){
    difference(){
        union(){
			cylinder(h=plateThickness,r=mountingtabwidth, $fn=12);
            translate([-mountingtabwidth,0,0]){
                cube([(mountingtabwidth)*2,
                       mountingtabwidth,
                       plateThickness]);
            }
        }
        cylinder(h=plateThickness,r=mountingholeradius, $fn=12);
    }
}

module plate(w,h){
    translate([-1,0,0])
	cube([w+2,h,plateThickness]);
    translate([w*taboffsetratio,h+mountingtabwidth,0]){
        rotate([0,0,180]){
            extendedMountingHole();
        }
    }
    translate([w-w*taboffsetratio,-mountingtabwidth,0]){
        extendedMountingHole();
    }
}

module switchhole(){
	union(){
		cube([holesize,holesize,plateThickness]);

		translate([-cutoutwidth,1,0])
		cube([holesize+2*cutoutwidth,cutoutheight,plateThickness]);

		translate([-cutoutwidth,holesize-cutoutwidth-cutoutheight,0])
		cube([holesize+2*cutoutwidth,cutoutheight,plateThickness]);
	}
}

module holematrix(holes,startx,starty){
	for (key = holes){
		translate([startx+lkey*key[0][0], starty-lkey*key[0][1], 0])
		translate([(lkey*key[1]-holesize)/2,(lkey - holesize)/2, 0])
		switchhole();
	}
}

module mountingholes(){
	translate([(1+1/3)*lkey,3.5*lkey,0])
	cylinder(h=plateThickness,r=mountingholeradius, $fn=12);

	translate([(13+2/3)*lkey,3.5*lkey,0])
	cylinder(h=plateThickness,r=mountingholeradius, $fn=12);
	
	translate([(6.75)*lkey,2.5*lkey,0])
	cylinder(h=plateThickness,r=mountingholeradius, $fn=12);

	translate([(6.75)*lkey,2.5*lkey,0])
	cylinder(h=plateThickness,r=mountingholeradius, $fn=12);

	translate([(14.8)*lkey,2*lkey,0])
	cylinder(h=plateThickness,r=mountingholeradius, $fn=12);

	translate([(.2)*lkey,2*lkey,0])
	cylinder(h=plateThickness,r=mountingholeradius, $fn=12);

	translate([(10)*lkey,.5*lkey,0])
	cylinder(h=plateThickness,r=mountingholeradius, $fn=12);
}

module myplate(){
	difference(){
		plate(w,h);
		holematrix(myKeyboard,0,h-lkey);
        // Mounting holes are now included in the plate.
		//mountingholes();
		//translate([152.5,0,0]) cube([.001,150,150]);
	}
}

module pokerplate(){
	difference(){
		plate(w,h);
		holematrix(pokerkeyboard,0,h-lkey);
		mountingholes();
	}
}

pokerplate();
