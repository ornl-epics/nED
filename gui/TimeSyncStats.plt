<?xml version="1.0" encoding="UTF-8"?>
<databrowser>
  <title>Time Sync Statistics</title>
  <save_changes>true</save_changes>
  <show_toolbar>true</show_toolbar>
  <update_period>3.0</update_period>
  <scroll_step>5</scroll_step>
  <scroll>true</scroll>
  <start>-10 minutes</start>
  <end>now</end>
  <archive_rescale>STAGGER</archive_rescale>
  <foreground>
    <red>0</red>
    <green>0</green>
    <blue>0</blue>
  </foreground>
  <background>
    <red>255</red>
    <green>255</green>
    <blue>255</blue>
  </background>
  <title_font>
  </title_font>
  <label_font>
  </label_font>
  <scale_font>
  </scale_font>
  <legend_font>
  </legend_font>
  <axes>
    <axis>
      <visible>true</visible>
      <name>Sync</name>
      <use_axis_name>false</use_axis_name>
      <use_trace_names>true</use_trace_names>
      <right>false</right>
      <color>
        <red>0</red>
        <green>0</green>
        <blue>0</blue>
      </color>
      <min>-100.0</min>
      <max>100.0</max>
      <grid>true</grid>
      <autoscale>true</autoscale>
      <log_scale>false</log_scale>
    </axis>
    <axis>
      <visible>false</visible>
      <name>PI</name>
      <use_axis_name>false</use_axis_name>
      <use_trace_names>true</use_trace_names>
      <right>true</right>
      <color>
        <red>0</red>
        <green>0</green>
        <blue>0</blue>
      </color>
      <min>0.0</min>
      <max>5.0</max>
      <grid>false</grid>
      <autoscale>false</autoscale>
      <log_scale>false</log_scale>
    </axis>
    <axis>
      <visible>false</visible>
      <name>GPS</name>
      <use_axis_name>false</use_axis_name>
      <use_trace_names>true</use_trace_names>
      <right>true</right>
      <color>
        <red>0</red>
        <green>0</green>
        <blue>0</blue>
      </color>
      <min>0.0</min>
      <max>10.0</max>
      <grid>false</grid>
      <autoscale>true</autoscale>
      <log_scale>false</log_scale>
    </axis>
  </axes>
  <annotations>
  </annotations>
  <pvlist>
    <pv>
      <display_name>Remote Time Offset</display_name>
      <visible>true</visible>
      <name>$(P)TS:RemoteTimeOff</name>
      <axis>0</axis>
      <color>
        <red>0</red>
        <green>0</green>
        <blue>255</blue>
      </color>
      <trace_type>AREA</trace_type>
      <linewidth>2</linewidth>
      <point_type>SQUARES</point_type>
      <point_size>2</point_size>
      <waveform_index>0</waveform_index>
      <period>0.0</period>
      <ring_size>5000</ring_size>
      <request>OPTIMIZED</request>
    </pv>
    <pv>
      <display_name>Comms Delay</display_name>
      <visible>true</visible>
      <name>$(P)TS:CommDly</name>
      <axis>0</axis>
      <color>
        <red>0</red>
        <green>255</green>
        <blue>255</blue>
      </color>
      <trace_type>AREA</trace_type>
      <linewidth>2</linewidth>
      <point_type>NONE</point_type>
      <point_size>2</point_size>
      <waveform_index>0</waveform_index>
      <period>0.0</period>
      <ring_size>5000</ring_size>
      <request>OPTIMIZED</request>
    </pv>
    <pv>
      <display_name>PI input error</display_name>
      <visible>false</visible>
      <name>$(P)TS:SyncInErr</name>
      <axis>1</axis>
      <color>
        <red>255</red>
        <green>204</green>
        <blue>102</blue>
      </color>
      <trace_type>AREA</trace_type>
      <linewidth>2</linewidth>
      <point_type>NONE</point_type>
      <point_size>2</point_size>
      <waveform_index>0</waveform_index>
      <period>0.0</period>
      <ring_size>5000</ring_size>
      <request>OPTIMIZED</request>
    </pv>
    <pv>
      <display_name>PI adjusted error</display_name>
      <visible>false</visible>
      <name>$(P)TS:SyncAdjErr</name>
      <axis>1</axis>
      <color>
        <red>255</red>
        <green>127</green>
        <blue>0</blue>
      </color>
      <trace_type>AREA</trace_type>
      <linewidth>2</linewidth>
      <point_type>NONE</point_type>
      <point_size>2</point_size>
      <waveform_index>0</waveform_index>
      <period>0.0</period>
      <ring_size>5000</ring_size>
      <request>OPTIMIZED</request>
    </pv>
    <pv>
      <display_name>PI output error</display_name>
      <visible>false</visible>
      <name>$(P)TS:SyncOutErr</name>
      <axis>1</axis>
      <color>
        <red>255</red>
        <green>0</green>
        <blue>0</blue>
      </color>
      <trace_type>AREA</trace_type>
      <linewidth>2</linewidth>
      <point_type>NONE</point_type>
      <point_size>2</point_size>
      <waveform_index>0</waveform_index>
      <period>0.0</period>
      <ring_size>5000</ring_size>
      <request>OPTIMIZED</request>
    </pv>
    <pv>
      <display_name>GPS offset</display_name>
      <visible>false</visible>
      <name>$(P)TS:GpsTimeOff</name>
      <axis>2</axis>
      <color>
        <red>0</red>
        <green>255</green>
        <blue>0</blue>
      </color>
      <trace_type>AREA</trace_type>
      <linewidth>2</linewidth>
      <point_type>NONE</point_type>
      <point_size>2</point_size>
      <waveform_index>0</waveform_index>
      <period>0.0</period>
      <ring_size>5000</ring_size>
      <request>OPTIMIZED</request>
      <archive>
        <name>Accelerator</name>
        <url>jdbc:oracle:thin:@(DESCRIPTION=(LOAD_BALANCE=OFF)(FAILOVER=ON)(ADDRESS=(PROTOCOL=TCP)(HOST=snsappa.sns.ornl.gov)(PORT=1610))(ADDRESS=(PROTOCOL=TCP)(HOST=snsappb.sns.ornl.gov)(PORT=1610))(CONNECT_DATA=(SERVICE_NAME=prod_controls)))</url>
      </archive>
      <archive>
        <name>Instruments</name>
        <url>jdbc:oracle:thin:@snsoroda-scan.sns.gov:1521/scprod_controls</url>
      </archive>
    </pv>
  </pvlist>
</databrowser>