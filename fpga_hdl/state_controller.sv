`define SPI_CMD_NOP 8'h0
`define SPI_CMD_WRITE 8'h1
`define SPI_CMD_READ_ADDR 8'h2
`define SPI_CMD_READ_DATA 8'h3
`define SPI_CMD_READ_READY 8'h4

module state_controller(
    input clk,
    input SCK,
    input MOSI,
    output MISO,
    input SSEL,

    output [23:0] wr_addr,
    output [15:0] wr_data,
    output wr_enable,

    output [23:0] rd_addr,
    input [15:0] rd_data,
    input rd_ready,
    output rd_enable,

    input busy,
    output rst_n,
    output led
);

enum {
    state_begin_1=0,
    state_begin_2,
    state_begin_3,
    state_begin_4,
    state_begin_5,
    state_begin_6,
    state_begin_7,
    state_wait,
    state_data_write_addr_1,
    state_data_write_addr_2,
    state_data_write_addr_3,
    state_data_write_data_1,
    state_data_write_data_2,
    state_data_write_commit,
    state_data_write_done,
    state_data_read_addr_1_pre,
    state_data_read_addr_1,
    state_data_read_addr_2,
    state_data_read_addr_3,
    state_data_read_data_1,
    state_data_read_data_2,
    state_data_read_done,
    state_data_read_done_commit,
    state_data_read_done_commit_1,
    state_data_read_done_commit_2,
    state_data_read_set_1,
    state_data_read_set_2,
    state_data_read_set_3,
    state_data_read_set_4,
    state_data_read_ready_1,
    state_data_read_ready_2
} state;

logic spi_data_ready;
logic [7:0] spi_data_recv;
logic [7:0] spi_data_send;
logic [2:0] spi_bit_count;
logic [23:0] data_addr;
logic [15:0] data;

logic [23:0] ram_wr_addr;
logic [15:0] ram_wr_data;
logic ram_wr_enable;
assign wr_addr = ram_wr_addr;
assign wr_data = ram_wr_data;
assign wr_enable = ram_wr_enable;

logic [23:0] ram_rd_addr;
logic ram_rd_enable;
assign rd_addr = ram_rd_addr;
assign rd_enable = ram_rd_enable;

logic ram_rst;
assign rst_n = ram_rst;

logic led_out;
assign led = led_out;

logic spi_busy;
logic spi_do_send;

logic read_good;

spi slave_spi(
    .clk(clk),
    .SCK(SCK),
    .MOSI(MOSI),
    .MISO(MISO),
    .SSEL(SSEL),
    .data_ready(spi_data_ready),
    .data_recv(spi_data_recv),
    .data_send(spi_data_send),
    .bit_count_out(spi_bit_count),
    .do_send(spi_do_send),
    .busy(spi_busy));

initial begin
    state = state_begin_1;
end


always @(posedge clk) begin
    if(rd_ready == 1 && read_good == 0) begin
        data <= rd_data;
        read_good <= 1;
    end
    case(state)
    state_begin_1: begin
        ram_rst <= 0;
        led_out <= 0;
        state <= state_begin_2;
    end
    state_begin_2: begin
        ram_rst <= 1;
        state <= state_begin_3;
    end
    state_begin_3: begin
        if(!busy) begin
            ram_wr_addr <= 24'h000001;
            ram_wr_data <= 16'hDEAD;
            ram_wr_enable <= 1;
            state <= state_begin_4;
        end
    end
    state_begin_4: begin
        if(busy) begin
            ram_wr_enable <= 0;
            state <= state_begin_5;
        end
    end
    state_begin_5: begin
        if(!busy) begin
            ram_rd_addr <= 24'h000001;
            ram_rd_enable <= 1;
            state <= state_begin_6;
        end
    end
    state_begin_6: begin
        if(busy) begin
            ram_rd_enable <= 0;
            state <= state_begin_7;
        end
    end
    state_begin_7: begin
        if(rd_ready) begin
            if(rd_data == 16'hDEAD) begin 
					led_out <= 1;
            end else begin
                led_out <= 0;
            end
            state <= state_wait;
        end
    end
    state_wait: begin
        if(spi_data_ready) begin
            if(spi_data_recv == `SPI_CMD_WRITE) begin
                state <= state_data_write_addr_1;
            end else if(spi_data_recv == `SPI_CMD_READ_ADDR) begin
                data <= 0;
                state <= state_data_read_addr_1;
            end else if(spi_data_recv == `SPI_CMD_READ_DATA) begin
                state <= state_data_read_set_1;
            end else if(spi_data_recv == `SPI_CMD_READ_READY) begin
                state <= state_data_read_ready_1;
            end
        end
    end
    state_data_write_addr_1: begin
        if(spi_data_ready) begin
            data_addr[23:16] <= spi_data_recv;
            state <= state_data_write_addr_2;
        end
    end
    state_data_write_addr_2: begin
        if(spi_data_ready) begin
            data_addr[15:8] <= spi_data_recv;
            state <= state_data_write_addr_3;
        end
    end
    state_data_write_addr_3: begin
        if(spi_data_ready) begin
            data_addr[7:0] <= spi_data_recv;
            state <= state_data_write_data_1;
        end
    end
    state_data_write_data_1: begin
        if(spi_data_ready) begin
            data[15:8] <= spi_data_recv;
            state <= state_data_write_data_2;
        end
    end
    state_data_write_data_2: begin
        if(spi_data_ready) begin
            data[7:0] <= spi_data_recv;
            state <= state_data_write_commit;
        end
    end
    state_data_write_commit: begin
        if(!busy) begin
            ram_wr_addr <= data_addr;
            ram_wr_data <= data;
            ram_wr_enable <= 1;
            state <= state_data_write_done;
        end
    end
    state_data_write_done: begin
        if(busy) begin
            ram_wr_enable <= 0;
            state <= state_wait;
        end
    end
    /* Set read addr command */
    state_data_read_addr_1: begin
        if(spi_data_ready) begin
            data_addr[23:16] <= spi_data_recv;
            state <= state_data_read_addr_2;
        end    
    end
    state_data_read_addr_2: begin
        if(spi_data_ready) begin
            data_addr[15:8] <= spi_data_recv;
            state <= state_data_read_addr_3;
        end    
    end
    state_data_read_addr_3: begin
        if(spi_data_ready) begin
            data_addr[7:0] <= spi_data_recv;
            state <= state_data_read_data_1;
        end    
    end
    state_data_read_data_1: begin
        if(!busy) begin
            ram_rd_addr <= data_addr;
            ram_rd_enable <= 1;
            state <= state_data_read_data_2;
        end    
    end
    state_data_read_data_2: begin
        if(busy) begin
            ram_rd_enable <= 0;
            read_good <= 0;
            state <= state_wait;
        end    
    end
    state_data_read_set_1: begin
        spi_data_send <= data[15:8];
        spi_do_send <= 1;
        state <= state_data_read_set_2;
    end
    state_data_read_set_2: begin
        if(spi_busy == 1) begin
            spi_do_send <= 0;
            state <= state_data_read_set_3;
        end
    end
    state_data_read_set_3: begin
        if(spi_busy == 0) begin
            spi_data_send <= data[7:0];
            spi_do_send <= 1;
            state <= state_data_read_set_4;
        end
    end
    state_data_read_set_4: begin
        if(spi_busy == 1) begin
            spi_do_send <= 0;
            state <= state_wait;
        end
    end
    state_data_read_ready_1: begin
        if(read_good) begin
            spi_data_send <= 8'h01;
            spi_do_send <= 1;
            state <= state_data_read_ready_2;
        end else begin
            spi_data_send <= 8'hFF;
            spi_do_send <= 1;
            state <= state_data_read_ready_2;
        end
    end
    state_data_read_ready_2: begin
        if(spi_busy == 1) begin
            spi_do_send <= 0;
            state <= state_wait;
        end
    end
    endcase
end

endmodule