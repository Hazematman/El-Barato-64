`define SPI_CMD_NOP 8'h0
`define SPI_CMD_WRITE 8'h1
`define SPI_CMD_READ_ADDR 8'h2
`define SPI_CMD_READ_DATA 8'h3

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
    output rst_n
);

enum {
    state_begin_1,
    state_begin_2,
    state_wait,
    state_data_write_addr_1,
    state_data_write_addr_2,
    state_data_write_addr_3,
    state_data_write_data_1,
    state_data_write_data_2,
    state_data_write_commit,
    state_data_write_done,
    state_data_read_addr_1,
    state_data_read_addr_2,
    state_data_read_addr_3,
    state_data_read_data_1,
    state_data_read_data_2,
    state_data_read_done,
    state_data_read_set_1,
    state_data_read_set_2
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

spi slave_spi(
    .clk(clk),
    .SCK(SCK),
    .MOSI(MOSI),
    .MISO(MISO),
    .SSEL(SSEL),
    .data_ready(spi_data_ready),
    .data_recv(spi_data_recv),
    .data_send(spi_data_send),
    .bit_count_out(spi_bit_count));

initial begin
    state = state_begin_1;
end


always @(posedge clk) begin
    case(state)
    state_begin_1: begin
        ram_rst <= 0;
        state <= state_begin_2;
    end
    state_begin_2: begin
        ram_rst <= 1;
        state <= state_wait;
    end
    state_wait: begin
        if(spi_data_ready) begin
            if(spi_data_recv == `SPI_CMD_WRITE) begin
                state <= state_data_write_addr_1;
            end else if(spi_data_recv == `SPI_CMD_READ_ADDR) begin
                state <= state_data_read_addr_1;
            end else if(spi_data_recv == `SPI_CMD_READ_DATA) begin
                state <= state_data_read_set_1;
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
            state <= state_data_read_done;
        end    
    end
    state_data_read_done: begin
        if(rd_ready) begin
            data <= rd_data;
            ram_rd_enable <= 0;
            state <= state_wait;
        end    
    end
    state_data_read_set_1: begin
        spi_data_send <= data[15:8];
        state <= state_data_read_set_2;
    end
    state_data_read_set_2: begin
        if(spi_bit_count == 3'b111) begin
            spi_data_send <= data[7:0];
            state <= state_wait;
        end
    end
    endcase
end

endmodule