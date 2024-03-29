Library ieee;
Use ieee.std_logic_1164.all;
Use ieee.std_logic_unsigned.all;
Use ieee.std_logic_arith.all;

Entity iis is
  generic(	--bck rate
			n: integer:=32  --lrck rate
  );
  port(
        rstin: IN std_logic;--1:resrt ,0:working
        clkin45: IN std_logic;--45.1584mhz
        clkin49: IN std_logic;--49.152mhz
        clkfsel: IN std_logic;--0:sel 45, 1: sel 49
        clks0: IN std_logic;--00:/1  01:/2  10:/4  11:/8
        clks1: IN std_logic;
        mclk2: OUT std_logic;
        


        bckcontrol: IN std_logic;---for usb boot,normal:1,usb:0,when usb+reset,bck forced to 0.        
        bckout: OUT std_logic;--MUST TO MCU BCK!
        lrckout: OUT std_logic;
        bckout2: OUT std_logic;--out port
        lrckout2: OUT std_logic;
        
        mcu_o: IN std_logic;
        mcu_i: OUT std_logic;
        f3_i: IN std_logic;
        dat_o: OUT std_logic
        );
End iis;

Architecture a of iis is
  
  signal clkin:std_logic;--switched 45/49mhz 
  signal clkdiva:std_logic;--22.5792mhz
  signal clkdivb:std_logic;--11.2896mhz
  signal clkdivc:std_logic;--5.6448mhz
  signal mclk128fs: std_logic;----divided mclk
  signal bclk: std_logic;
  signal lrclk: std_logic; 
  
  signal cntl: integer range 0 to n-1; 

Begin

----45/49 clock switch
process(clkin45,clkin49,clkfsel) begin
	if(clkfsel='1') then
		clkin<=clkin49;
	else
		clkin<=clkin45;
	end if;
end process;

----clock dividers
  
---- 1st  /2 divider ,in=45 out=22
process(clkin,rstin) 
begin
	if(clkin'event and clkin='1') then ---������2��Ƶ
		clkdiva<=not clkdiva;
	end if;
end process;

------2nd  /2 divider,in=22,out=11
process(clkdiva,rstin) 
begin
	if(clkdiva'event and clkdiva='1') then ---������2��Ƶ
		clkdivb<=not clkdivb;
	end if;
end process;
------3rd  /2 divider,in=11 out=5
process(clkdivb,rstin) 
begin
	if(clkdivb'event and clkdivb='1') then ---������2��Ƶ
		clkdivc<=not clkdivc;
	end if;
end process;

----MCLK2 clock divider selecter
process(clks0,clks1,clkin,clkdiva,clkdivb,clkdivc)
begin
	if (clks0='0' and clks1='0')then
		mclk2<=clkin;
	elsif (clks0='0' and clks1='1')then
		mclk2<=clkin;
	elsif (clks0='1' and clks1='0')then
		mclk2<=clkdiva;
	else  ---(clks0='1' and clks1='1')
		mclk2<=clkdivb;
	end if;
end process;


----ʱ��ѡ���� clock divider selecter
process(clks0,clks1,clkin,clkdiva,clkdivb,clkdivc)
begin
	if (clks0='0' and clks1='0')then
		mclk128fs<=clkin;
	elsif (clks0='0' and clks1='1')then
		mclk128fs<=clkdiva;
	elsif (clks0='1' and clks1='0')then
		mclk128fs<=clkdivb;
	else  ---(clks0='1' and clks1='1')
		mclk128fs<=clkdivc;
	end if;
end process;


----BCK : mclk=128fs ,bck=64fs ,so ,mclk/2=bck
process(mclk128fs,rstin) 
begin
	if(rstin='1') then --�첽��λ
		bclk <= '0';---bck start at logic 0
	elsif(mclk128fs'event and mclk128fs='1') then ---������2��Ƶ
		bclk<=not bclk;
	end if;
end process;


---LRCK: 64bit per frame
--- bck /64 =lrck
process(bclk,lrclk,rstin) 
begin


	if (rstin='1')then --�첽��λ
			cntl <= 0;
			lrclk <= '1';  --lrck start at left channel,logic 1 
  
	elsif(bclk'event and bclk='0') then

		if(cntl<n-1) then
			cntl <= cntl+1;
		else
			cntl <= 0;
			lrclk <= not lrclk;
		end if;
	end if;

end process;

-- bck lrck io
process(bclk,lrclk,rstin,bckcontrol)  
begin
	if (rstin='1')then
		bckout2 <='0';
		bckout <= bckcontrol;		
	else
		bckout2 <= bclk;
		bckout <= bclk;
	end if;
	
	lrckout2 <= lrclk;
	lrckout <= lrclk;
		
end process;


---pass-through io
process(mcu_o,f3_i)  
begin

		dat_o <= mcu_o;
		mcu_i <= f3_i;

end process;

End a;