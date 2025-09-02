/**
 * @brief Envía las métricas recolectadas como un objeto JSON a través de un
 * pipe.
 */
#include "cjson_metrics.h"
#include "../../../lib/memory/include/memory.h"

void send_metrics_as_json()
{
    cJSON* root = cJSON_CreateObject();

    // Recolectar las métricas de CPU, memoria, disco, red, procesos y cambios de
    // contexto
    CpuUsageState state = {0,0,0,0,0,0,0,0};
    double cpu_usage = get_cpu_usage(&state);
    double memory_usage = get_memory_usage();
    struct DiskStats disk_stats = get_disk_io();
    struct NetStats network_stats = get_network_stats();
    int running_processes = get_process_count();
    unsigned long long context_switches = get_context_switches();

    // Recolectar métricas de memoria personalizada
    struct MemStats custom_mem_stats;
    int mem_stats_available = (calculate_mem_stats(&custom_mem_stats) == 0);

    // Añadir métricas al objeto JSON
    cJSON_AddNumberToObject(root, "cpu_usage_percentage", (double)cpu_usage);
    cJSON_AddNumberToObject(root, "memory_usage_percentage", (double)memory_usage);
    cJSON_AddNumberToObject(root, "disk_reads", (double)disk_stats.reads);
    cJSON_AddNumberToObject(root, "disk_writes", (double)disk_stats.writes);
    cJSON_AddNumberToObject(root, "disk_read_time_seconds", (double)disk_stats.read_time);
    cJSON_AddNumberToObject(root, "disk_write_time_seconds", (double)disk_stats.write_time);
    cJSON_AddNumberToObject(root, "network_bandwidth_rx", (double)network_stats.bytes_received);
    cJSON_AddNumberToObject(root, "network_bandwidth_tx", (double)network_stats.bytes_transmitted);
    cJSON_AddNumberToObject(root, "network_packet_ratio",
                            (double)(network_stats.packets_received > 0)
                                ? (double)network_stats.packets_transmitted / (double)network_stats.packets_received
                                : 0.0);
    cJSON_AddNumberToObject(root, "running_processes_count", (double)running_processes);
    cJSON_AddNumberToObject(root, "context_switches_total", (double)context_switches);

    
    // Añadir métricas de memoria personalizada
    if (mem_stats_available) {
        cJSON_AddNumberToObject(root, "custom_memory_allocated_bytes", (double)custom_mem_stats.total_allocated);
        cJSON_AddNumberToObject(root, "custom_memory_free_bytes", (double)custom_mem_stats.total_free);
        cJSON_AddNumberToObject(root, "custom_memory_fragmentation_percentage", custom_mem_stats.fragmentation * 100.0);
        cJSON_AddNumberToObject(root, "custom_memory_largest_free_block_bytes", (double)custom_mem_stats.largest_free_block);
        cJSON_AddStringToObject(root, "custom_memory_policy", get_policy_name(custom_mem_stats.most_efficient_policy));
        
        // Calcular eficiencia
        size_t total_memory = custom_mem_stats.total_allocated + custom_mem_stats.total_free;
        double efficiency = (total_memory > 0) ? (double)custom_mem_stats.total_free / (double)total_memory : 0.0;
        cJSON_AddNumberToObject(root, "custom_memory_efficiency_ratio", efficiency);
    } else {
        // Valores por defecto si no hay estadísticas disponibles
        cJSON_AddNumberToObject(root, "custom_memory_allocated_bytes", 0.0);
        cJSON_AddNumberToObject(root, "custom_memory_free_bytes", 0.0);
        cJSON_AddNumberToObject(root, "custom_memory_fragmentation_percentage", 0.0);
        cJSON_AddNumberToObject(root, "custom_memory_largest_free_block_bytes", 0.0);
        cJSON_AddStringToObject(root, "custom_memory_policy", "UNKNOWN");
        cJSON_AddNumberToObject(root, "custom_memory_efficiency_ratio", 0.0);
    }


    // Convertir el objeto JSON a string y enviarlo al pipe
    char* json_data = cJSON_Print(root);

    if (json_data != NULL)
    {
        FILE* pipe = fopen("/tmp/monitor_pipe",
                           "w"); // "a" mantiene el pipe abierto para múltiples escrituras
        if (pipe)
        {
            fprintf(pipe, "%s\n", json_data);
            fflush(pipe); // Asegura que los datos se escriban inmediatamente
            fclose(pipe);
        }
        else
        {
            perror("Error al abrir el pipe para enviar métricas");
        }

        free(json_data);
    }

    // Limpiar el objeto JSON
    cJSON_Delete(root);
}