using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.Json;
using System.Security.Cryptography;
using System.Text;
using System.Globalization; // For parsing dates

namespace ECommerceInventoryManager
{
    // Models for the application
    public class UserActivity
    {
        public DateTime Timestamp { get; set; }
        public string ActivityType { get; set; }
        public string Description { get; set; }

        public UserActivity()
        {
            Timestamp = DateTime.Now;
        }

        public UserActivity(string activityType, string description) : this()
        {
            ActivityType = activityType;
            Description = description;
        }

        public override string ToString()
        {
            return $"{Timestamp}: {ActivityType} - {Description}";
        }
    }

    public class User
    {
        public string Username { get; set; }
        public string PasswordHash { get; set; }
        public string Role { get; set; } // Admin, Manager, Staff
        public bool IsActive { get; set; } = true;
        public DateTime LastLogin { get; set; }
        public List<UserActivity> ActivityLog { get; set; } = new List<UserActivity>();

        // Default constructor for deserialization
        public User() { }

        public User(string username, string password, string role)
        {
            Username = username;
            PasswordHash = HashPassword(password);
            Role = role;
            LastLogin = DateTime.MinValue;
        }

        public static string HashPassword(string password)
        {
            using (SHA256 sha256 = SHA256.Create())
            {
                // Convert the input string to a byte array and compute the hash
                byte[] data = sha256.ComputeHash(Encoding.UTF8.GetBytes(password));

                // Create a new StringBuilder to collect the bytes and create a string
                var sBuilder = new StringBuilder();

                // Loop through each byte of the hashed data and format each one as a hexadecimal string
                for (int i = 0; i < data.Length; i++)
                {
                    sBuilder.Append(data[i].ToString("x2"));
                }

                // Return the hexadecimal string
                return sBuilder.ToString();
            }
        }

        public bool VerifyPassword(string password)
        {
            return PasswordHash == HashPassword(password);
        }

        public void LogActivity(string activityType, string description)
        {
            if (ActivityLog == null) ActivityLog = new List<UserActivity>();
            ActivityLog.Add(new UserActivity(activityType, description));
        }
    }

    public class Supplier
    {
        public string Id { get; private set; }
        public string Name { get; set; }
        public string ContactPerson { get; set; }
        public string Email { get; set; }
        public string Phone { get; set; }
        public string Address { get; set; }
        public bool IsActive { get; set; } = true;
        public DateTime LastOrderDate { get; set; } = DateTime.MinValue;
        public int TotalOrders { get; set; } = 0;
        public int LateDeliveries { get; set; } = 0;
        public decimal AverageLeadTimeDays { get; set; } = 0;

        public Supplier()
        {
            // Generate a unique ID for new supplier
            Id = $"SUP-{Guid.NewGuid().ToString().Substring(0, 8).ToUpper()}";
        }

        public override string ToString()
        {
            return $"{Id}: {Name} | {(IsActive ? "Active" : "Inactive")} | Last Order: {(LastOrderDate == DateTime.MinValue ? "Never" : LastOrderDate.ToShortDateString())}";
        }
    }

    public class PurchaseOrderItem
    {
        public string ProductId { get; set; }
        public string ProductName { get; set; }
        public decimal CostPrice { get; set; }
        public int Quantity { get; set; }

        public decimal GetTotal()
        {
            return CostPrice * Quantity;
        }
    }

    public class PurchaseOrder
    {
        public string PoId { get; private set; }
        public string SupplierId { get; set; }
        public string SupplierName { get; set; }
        public DateTime OrderDate { get; private set; }
        public DateTime? ExpectedDeliveryDate { get; set; }
        public DateTime? ActualDeliveryDate { get; set; }
        public string Status { get; set; } // Draft, Submitted, Received, Cancelled
        public List<PurchaseOrderItem> Items { get; set; } = new List<PurchaseOrderItem>();

        public PurchaseOrder()
        {
            // Generate a unique ID for new PO
            PoId = $"PO-{Guid.NewGuid().ToString().Substring(0, 8).ToUpper()}";
            OrderDate = DateTime.Now;
        }

        public decimal GetTotal()
        {
            if (Items == null || !Items.Any()) return 0;
            return Items.Sum(i => i.GetTotal());
        }

        public override string ToString()
        {
            return $"{PoId}: Supplier: {SupplierName} | Date: {OrderDate.ToShortDateString()} | Status: {Status} | Items: {Items.Count} | Total: ${GetTotal():F2}";
        }
    }

    public class Product
    {
        public string Id { get; private set; }
        public string Name { get; set; }
        public string Description { get; set; }
        public string Category { get; set; }
        public decimal Price { get; set; }
        public decimal CostPrice { get; set; }
        public int Quantity { get; set; }
        public bool IsActive { get; set; } = true;
        public decimal TaxRate { get; set; } = 0.10m; // Default tax rate 10%
        public decimal DiscountPercent { get; set; } = 0; // Default discount 0%
        public DateTime LastRestocked { get; set; } = DateTime.MinValue;
        public string SupplierId { get; set; }
        public string SupplierName { get; set; }

        public Product()
        {
            // Generate a unique ID for new product
            Id = $"PRD-{Guid.NewGuid().ToString().Substring(0, 8).ToUpper()}";
        }

        public decimal GetDiscountedPrice()
        {
            return Price * (1 - (DiscountPercent / 100m));
        }

        public override string ToString()
        {
            return $"{Id}: {Name} | Category: {Category} | Stock: {Quantity} | Price: ${Price:F2} | {(IsActive ? "Active" : "Inactive")}";
        }
    }

    public class OrderItem
    {
        public string ProductId { get; set; }
        public string ProductName { get; set; }
        public decimal Price { get; set; }
        public decimal CostPrice { get; set; }
        public int Quantity { get; set; }

        public decimal GetTotal()
        {
            return Price * Quantity;
        }

        public decimal GetProfit()
        {
            return (Price - CostPrice) * Quantity;
        }
    }

    public class Order
    {
        public string OrderId { get; private set; }
        public string CustomerName { get; set; }
        public DateTime OrderDate { get; private set; }
        public string Status { get; set; } // Pending, Processing, Shipped, Completed, Cancelled
        public List<OrderItem> Items { get; set; } = new List<OrderItem>();
        public decimal TaxRate { get; set; } = 0.10m; // Default tax rate 10%
        public decimal DiscountAmount { get; set; } = 0; // Flat discount amount
        public string PaymentMethod { get; set; } = "Cash";
        public bool IsPaid { get; set; } = false;

        public Order()
        {
            // Generate a unique ID for new order
            OrderId = $"ORD-{Guid.NewGuid().ToString().Substring(0, 8).ToUpper()}";
            OrderDate = DateTime.Now;
            Status = "Pending";
        }

        public decimal GetSubtotal()
        {
            if (Items == null || !Items.Any()) return 0;
            return Items.Sum(i => i.GetTotal());
        }

        public decimal GetTaxAmount()
        {
            return (GetSubtotal() - DiscountAmount) * TaxRate;
        }

        public decimal GetTotal()
        {
            return GetSubtotal() - DiscountAmount + GetTaxAmount();
        }

        public decimal GetTotalCost()
        {
            if (Items == null || !Items.Any()) return 0;
            return Items.Sum(i => i.CostPrice * i.Quantity);
        }

        public decimal GetProfit()
        {
            return GetSubtotal() - GetTotalCost() - DiscountAmount;
        }

        public override string ToString()
        {
            return $"{OrderId}: {CustomerName} | Date: {OrderDate.ToShortDateString()} | Status: {Status} | Items: {Items.Count} | Total: ${GetTotal():F2} | {(IsPaid ? "Paid" : "Unpaid")}";
        }
    }

    public static class Report
    {
        public static void GenerateInventoryReport(List<Product> products)
        {
            Console.WriteLine("\n===== INVENTORY REPORT =====");
            Console.WriteLine($"Generated on: {DateTime.Now}");
            Console.WriteLine($"Total Products: {products.Count}");

            if (!products.Any())
            {
                Console.WriteLine("No products in inventory.");
                return;
            }

            var lowStockThreshold = 5; // Consider anything below 5 to be "low stock"
            var lowStockItems = products.Where(p => p.IsActive && p.Quantity < lowStockThreshold).ToList();
            var outOfStockItems = products.Where(p => p.IsActive && p.Quantity == 0).ToList();
            var totalStockValue = products.Sum(p => p.Quantity * p.CostPrice);
            var totalRetailValue = products.Sum(p => p.Quantity * p.Price);

            Console.WriteLine($"Total Stock Value (Cost): ${totalStockValue:F2}");
            Console.WriteLine($"Total Retail Value: ${totalRetailValue:F2}");
            Console.WriteLine($"Potential Profit: ${totalRetailValue - totalStockValue:F2}");
            Console.WriteLine($"Average Product Price: ${(products.Any() ? products.Average(p => p.Price) : 0):F2}");
            Console.WriteLine($"Active Products: {products.Count(p => p.IsActive)}");
            Console.WriteLine($"Inactive Products: {products.Count(p => !p.IsActive)}");
            Console.WriteLine($"Products Out of Stock: {outOfStockItems.Count}");
            Console.WriteLine($"Products with Low Stock: {lowStockItems.Count} (below {lowStockThreshold} units)");

            if (lowStockItems.Any())
            {
                Console.WriteLine("\n--- Low Stock Items (Reorder Recommended) ---");
                foreach (var product in lowStockItems.OrderBy(p => p.Quantity))
                {
                    Console.WriteLine($"{product.Id}: {product.Name} | Current Stock: {product.Quantity} | Category: {product.Category}");
                }
            }

            Console.WriteLine("\n--- Most Valuable Items in Stock (Top 10) ---");
            foreach (var product in products.Where(p => p.IsActive && p.Quantity > 0)
                                           .OrderByDescending(p => p.Price * p.Quantity)
                                           .Take(10))
            {
                Console.WriteLine($"{product.Id}: {product.Name} | Stock: {product.Quantity} | Value: ${product.Price * product.Quantity:F2}");
            }
        }

        public static void GenerateCategoryReport(List<Product> products)
        {
            Console.WriteLine("\n===== INVENTORY BY CATEGORY REPORT =====");
            Console.WriteLine($"Generated on: {DateTime.Now}");

            if (!products.Any())
            {
                Console.WriteLine("No products in inventory.");
                return;
            }

            var categories = products.Select(p => p.Category).Distinct().OrderBy(c => c).ToList();
            Console.WriteLine($"Total Categories: {categories.Count}");

            foreach (var category in categories)
            {
                var categoryProducts = products.Where(p => p.Category == category).ToList();
                var activeProducts = categoryProducts.Where(p => p.IsActive).ToList();
                var stockValue = categoryProducts.Sum(p => p.Quantity * p.CostPrice);
                var totalItems = categoryProducts.Sum(p => p.Quantity);

                Console.WriteLine($"\n--- Category: {category} ---");
                Console.WriteLine($"Products in Category: {categoryProducts.Count} ({activeProducts.Count} Active)");
                Console.WriteLine($"Total Items in Stock: {totalItems}");
                Console.WriteLine($"Stock Value: ${stockValue:F2}");

                Console.WriteLine("Top 5 Products by Stock Value:");
                foreach (var product in categoryProducts.OrderByDescending(p => p.Quantity * p.CostPrice)
                                                    .Take(5))
                {
                    Console.WriteLine($"  {product.Name} - {product.Quantity} units - ${product.Quantity * product.CostPrice:F2}");
                }
            }
        }

        public static void GenerateFinancialReport(List<Order> orders, DateTime startDate, DateTime endDate)
        {
            Console.WriteLine("\n===== FINANCIAL REPORT =====");
            Console.WriteLine($"Period: {startDate.ToShortDateString()} to {endDate.ToShortDateString()}");
            Console.WriteLine($"Generated on: {DateTime.Now}");

            var periodOrders = orders.Where(o => o.OrderDate >= startDate && o.OrderDate <= endDate).ToList();

            if (!periodOrders.Any())
            {
                Console.WriteLine("No orders found in the specified period.");
                return;
            }

            var totalOrderCount = periodOrders.Count;
            var completedOrderCount = periodOrders.Count(o => o.Status == "Completed");
            var cancelledOrderCount = periodOrders.Count(o => o.Status == "Cancelled");
            var totalRevenue = periodOrders.Where(o => o.Status != "Cancelled").Sum(o => o.GetTotal());
            var totalCost = periodOrders.Where(o => o.Status != "Cancelled").Sum(o => o.GetTotalCost());
            var totalProfit = periodOrders.Where(o => o.Status != "Cancelled").Sum(o => o.GetProfit());
            var totalTax = periodOrders.Where(o => o.Status != "Cancelled").Sum(o => o.GetTaxAmount());
            var totalDiscounts = periodOrders.Where(o => o.Status != "Cancelled").Sum(o => o.DiscountAmount);
            var averageOrderValue = totalOrderCount > 0 ? totalRevenue / totalOrderCount : 0;

            Console.WriteLine("\n--- Sales Overview ---");
            Console.WriteLine($"Total Orders: {totalOrderCount}");
            Console.WriteLine($"Completed Orders: {completedOrderCount}");
            Console.WriteLine($"Cancelled Orders: {cancelledOrderCount}");

            Console.WriteLine("\n--- Financial Summary ---");
            Console.WriteLine($"Total Revenue: ${totalRevenue:F2}");
            Console.WriteLine($"Total Cost of Goods: ${totalCost:F2}");
            Console.WriteLine($"Gross Profit: ${totalProfit:F2}");
            Console.WriteLine($"Profit Margin: {(totalRevenue > 0 ? (totalProfit / totalRevenue) * 100 : 0):F2}%");
            Console.WriteLine($"Average Order Value: ${averageOrderValue:F2}");
            Console.WriteLine($"Total Tax Collected: ${totalTax:F2}");
            Console.WriteLine($"Total Discounts Given: ${totalDiscounts:F2}");

            // Payment Method Analysis
            var paymentMethods = periodOrders.Where(o => o.Status != "Cancelled")
                                            .GroupBy(o => o.PaymentMethod)
                                            .Select(g => new {
                                                Method = g.Key,
                                                Count = g.Count(),
                                                Total = g.Sum(o => o.GetTotal())
                                            })
                                            .OrderByDescending(x => x.Total)
                                            .ToList();

            Console.WriteLine("\n--- Payment Method Analysis ---");
            foreach (var method in paymentMethods)
            {
                Console.WriteLine($"{method.Method}: {method.Count} orders, ${method.Total:F2} ({(totalRevenue > 0 ? (method.Total / totalRevenue) * 100 : 0):F2}%)");
            }

            // Best selling items
            var bestSellingItems = periodOrders.Where(o => o.Status != "Cancelled")
                                              .SelectMany(o => o.Items)
                                              .GroupBy(i => i.ProductId)
                                              .Select(g => new {
                                                  ProductId = g.Key,
                                                  ProductName = g.First().ProductName,
                                                  Quantity = g.Sum(i => i.Quantity),
                                                  Revenue = g.Sum(i => i.GetTotal()),
                                                  Profit = g.Sum(i => i.GetProfit())
                                              })
                                              .OrderByDescending(x => x.Quantity)
                                              .Take(10)
                                              .ToList();

            Console.WriteLine("\n--- Top 10 Best Selling Products (by Quantity) ---");
            foreach (var item in bestSellingItems)
            {
                Console.WriteLine($"{item.ProductName} - Sold: {item.Quantity} units, Revenue: ${item.Revenue:F2}, Profit: ${item.Profit:F2}");
            }

            // Most profitable items
            var mostProfitableItems = periodOrders.Where(o => o.Status != "Cancelled")
                                                .SelectMany(o => o.Items)
                                                .GroupBy(i => i.ProductId)
                                                .Select(g => new {
                                                    ProductId = g.Key,
                                                    ProductName = g.First().ProductName,
                                                    Profit = g.Sum(i => i.GetProfit()),
                                                    Quantity = g.Sum(i => i.Quantity)
                                                })
                                                .OrderByDescending(x => x.Profit)
                                                .Take(10)
                                                .ToList();

            Console.WriteLine("\n--- Top 10 Most Profitable Products ---");
            foreach (var item in mostProfitableItems)
            {
                Console.WriteLine($"{item.ProductName} - Profit: ${item.Profit:F2}, Sold: {item.Quantity} units");
            }
        }

        public static void GenerateSupplierReport(List<Supplier> suppliers, List<PurchaseOrder> purchaseOrders)
        {
            Console.WriteLine("\n===== SUPPLIER PERFORMANCE REPORT =====");
            Console.WriteLine($"Generated on: {DateTime.Now}");

            if (!suppliers.Any())
            {
                Console.WriteLine("No suppliers found.");
                return;
            }

            Console.WriteLine($"Total Suppliers: {suppliers.Count}");
            Console.WriteLine($"Active Suppliers: {suppliers.Count(s => s.IsActive)}");

            // Get suppliers ordered by most orders
            var topSuppliersByOrders = suppliers
                .OrderByDescending(s => s.TotalOrders)
                .Take(10)
                .ToList();

            Console.WriteLine("\n--- Top Suppliers by Order Volume ---");
            foreach (var supplier in topSuppliersByOrders)
            {
                Console.WriteLine($"{supplier.Name} - Orders: {supplier.TotalOrders}, Late Deliveries: {supplier.LateDeliveries}, Avg Lead Time: {supplier.AverageLeadTimeDays:F1} days");
            }

            // Most reliable suppliers (least late deliveries relative to total orders)
            var reliableSuppliers = suppliers
                .Where(s => s.TotalOrders >= 3) // Only include suppliers with enough orders for meaningful data
                .OrderBy(s => s.TotalOrders > 0 ? (double)s.LateDeliveries / s.TotalOrders : 0)
                .ThenBy(s => s.AverageLeadTimeDays)
                .Take(5)
                .ToList();

            Console.WriteLine("\n--- Most Reliable Suppliers (Lowest Late Delivery Rate) ---");
            foreach (var supplier in reliableSuppliers)
            {
                var lateRate = supplier.TotalOrders > 0 ? ((double)supplier.LateDeliveries / supplier.TotalOrders) * 100 : 0;
                Console.WriteLine($"{supplier.Name} - On-Time Rate: {100 - lateRate:F1}%, Orders: {supplier.TotalOrders}, Lead Time: {supplier.AverageLeadTimeDays:F1} days");
            }

            // Recent purchase orders
            var recentPOs = purchaseOrders
                .OrderByDescending(po => po.OrderDate)
                .Take(5)
                .ToList();

            Console.WriteLine("\n--- Most Recent Purchase Orders ---");
            foreach (var po in recentPOs)
            {
                Console.WriteLine($"{po.PoId} - Supplier: {po.SupplierName}, Date: {po.OrderDate.ToShortDateString()}, Status: {po.Status}, Value: ${po.GetTotal():F2}");
            }

            // Purchase order status summary
            var poStatusSummary = purchaseOrders
                .GroupBy(po => po.Status)
                .Select(g => new { Status = g.Key, Count = g.Count() })
                .OrderBy(x => x.Status)
                .ToList();

            Console.WriteLine("\n--- Purchase Order Status Summary ---");
            foreach (var status in poStatusSummary)
            {
                Console.WriteLine($"{status.Status}: {status.Count} orders");
            }

            // Spending by supplier
            var supplierSpending = purchaseOrders
                .Where(po => po.Status != "Cancelled")
                .GroupBy(po => po.SupplierId)
                .Select(g => new {
                    SupplierId = g.Key,
                    SupplierName = g.First().SupplierName,
                    TotalSpend = g.Sum(po => po.GetTotal()),
                    OrderCount = g.Count()
                })
                .OrderByDescending(x => x.TotalSpend)
                .Take(10)
                .ToList();

            Console.WriteLine("\n--- Top Suppliers by Spend ---");
            foreach (var spend in supplierSpending)
            {
                Console.WriteLine($"{spend.SupplierName} - Total Spend: ${spend.TotalSpend:F2}, Orders: {spend.OrderCount}");
            }
        }
    }

    // Main inventory manager class with enhanced features
    class InventoryManager
    {
        private List<Product> products = new List<Product>();
        private List<Order> orders = new List<Order>();
        private List<User> users = new List<User>();
        private List<Supplier> suppliers = new List<Supplier>();
        private List<PurchaseOrder> purchaseOrders = new List<PurchaseOrder>();
        private User currentUser = null;

        private const string ProductFileName = "products.json";
        private const string OrdersFileName = "orders.json";
        private const string UsersFileName = "users.json";
        private const string SuppliersFileName = "suppliers.json";
        private const string PurchaseOrdersFileName = "purchase_orders.json";

        // Load all data from files
        public void LoadData()
        {
            try
            {
                Console.WriteLine("Loading data...");
                if (File.Exists(ProductFileName))
                {
                    string json = File.ReadAllText(ProductFileName);
                    products = JsonSerializer.Deserialize<List<Product>>(json) ?? new List<Product>();
                    Console.WriteLine($"Loaded {products.Count} products.");
                }

                if (File.Exists(OrdersFileName))
                {
                    string json = File.ReadAllText(OrdersFileName);
                    orders = JsonSerializer.Deserialize<List<Order>>(json) ?? new List<Order>();
                    Console.WriteLine($"Loaded {orders.Count} orders.");
                }

                if (File.Exists(UsersFileName))
                {
                    string json = File.ReadAllText(UsersFileName);
                    users = JsonSerializer.Deserialize<List<User>>(json) ?? new List<User>();
                    Console.WriteLine($"Loaded {users.Count} users.");
                }

                if (File.Exists(SuppliersFileName))
                {
                    string json = File.ReadAllText(SuppliersFileName);
                    suppliers = JsonSerializer.Deserialize<List<Supplier>>(json) ?? new List<Supplier>();
                    Console.WriteLine($"Loaded {suppliers.Count} suppliers.");
                }

                if (File.Exists(PurchaseOrdersFileName))
                {
                    string json = File.ReadAllText(PurchaseOrdersFileName);
                    purchaseOrders = JsonSerializer.Deserialize<List<PurchaseOrder>>(json) ?? new List<PurchaseOrder>();
                    Console.WriteLine($"Loaded {purchaseOrders.Count} purchase orders.");
                }

                // Create default admin if no users exist
                if (users.Count == 0)
                {
                    users.Add(new User("admin", "admin123", "Admin"));
                    Console.WriteLine("Created default admin user (Username: admin, Password: admin123)");
                    // Save immediately so the admin user persists if the app crashes before normal save
                    SaveData();
                }
                Console.WriteLine("Data loading complete.");
            }
            catch (JsonException jsonEx)
            {
                Console.WriteLine($"Error loading data (JSON parsing failed): {jsonEx.Message}");
                Console.WriteLine("Check the format of your .json files. Starting with empty lists.");
                // Initialize lists to prevent null reference errors later
                products = new List<Product>();
                orders = new List<Order>();
                users = new List<User>();
                suppliers = new List<Supplier>();
                purchaseOrders = new List<PurchaseOrder>();
                // Ensure default admin exists if users list was corrupt
                if (!users.Any()) users.Add(new User("admin", "admin123", "Admin"));
            }
            catch (Exception ex)
            {
                Console.WriteLine($"An unexpected error occurred during data loading: {ex.Message}");
                // Optionally, decide whether to proceed with empty lists or terminate
            }
        }

        // Save all data to files
        public void SaveData()
        {
            try
            {
                var options = new JsonSerializerOptions { WriteIndented = true };

                string productsJson = JsonSerializer.Serialize(products, options);
                File.WriteAllText(ProductFileName, productsJson);

                string ordersJson = JsonSerializer.Serialize(orders, options);
                File.WriteAllText(OrdersFileName, ordersJson);

                string usersJson = JsonSerializer.Serialize(users, options);
                File.WriteAllText(UsersFileName, usersJson);

                string suppliersJson = JsonSerializer.Serialize(suppliers, options);
                File.WriteAllText(SuppliersFileName, suppliersJson);

                string poJson = JsonSerializer.Serialize(purchaseOrders, options);
                File.WriteAllText(PurchaseOrdersFileName, poJson);

                Console.WriteLine("All data saved successfully.");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error saving data: {ex.Message}");
            }
        }

        // User login
        public bool Login()
        {
            int attempts = 0;
            const int maxAttempts = 3;

            while (attempts < maxAttempts)
            {
                Console.WriteLine($"\n===== LOGIN (Attempt {attempts + 1}/{maxAttempts}) =====");
                Console.Write("Username: ");
                string username = Console.ReadLine();
                Console.Write("Password: ");
                // Basic password masking (optional, can be improved)
                string password = "";
                ConsoleKeyInfo key;
                do
                {
                    key = Console.ReadKey(true);
                    if (key.Key != ConsoleKey.Backspace && key.Key != ConsoleKey.Enter)
                    {
                        password += key.KeyChar;
                        Console.Write("*");
                    }
                    else
                    {
                        if (key.Key == ConsoleKey.Backspace && password.Length > 0)
                        {
                            password = password.Substring(0, (password.Length - 1));
                            Console.Write("\b \b");
                        }
                    }
                } while (key.Key != ConsoleKey.Enter);
                Console.WriteLine(); // New line after password entry

                var user = users.FirstOrDefault(u => u.Username.Equals(username, StringComparison.OrdinalIgnoreCase) && u.IsActive);
                if (user != null && user.VerifyPassword(password))
                {
                    currentUser = user;
                    currentUser.LastLogin = DateTime.Now;
                    currentUser.LogActivity("Login", "User logged in successfully");
                    Console.WriteLine($"\nWelcome, {currentUser.Username} ({currentUser.Role})!");
                    Console.WriteLine($"Last Login: {currentUser.LastLogin}");
                    return true;
                }

                Console.WriteLine("Invalid username or password, or account is inactive.");
                attempts++;
            }
            Console.WriteLine("Maximum login attempts reached. Exiting.");
            return false;
        }

        // Logout
        public void Logout()
        {
            if (currentUser != null)
            {
                currentUser.LogActivity("Logout", "User logged out");
                Console.WriteLine($"Logging out {currentUser.Username}...");
                currentUser = null;
            }
        }

        // Main Application Menu
        public void ShowMainMenu()
        {
            if (currentUser == null)
            {
                Console.WriteLine("Error: No user logged in.");
                return;
            }

            bool exit = false;
            while (!exit)
            {
                Console.WriteLine($"\n===== MAIN MENU (User: {currentUser.Username}, Role: {currentUser.Role}) =====");
                Console.WriteLine("1. Product Management");
                Console.WriteLine("2. Order Management");
                Console.WriteLine("3. Supplier Management"); // Available to Manager+
                Console.WriteLine("4. Reports"); // Available to Manager+
                if (currentUser.Role == "Admin")
                {
                    Console.WriteLine("5. User Management");
                }
                Console.WriteLine("9. Logout");
                Console.WriteLine("0. Save & Exit");
                Console.Write("\nEnter your choice: ");

                if (int.TryParse(Console.ReadLine(), out int choice))
                {
                    switch (choice)
                    {
                        case 1:
                            ManageProducts();
                            break;
                        case 2:
                            ManageOrders();
                            break;
                        case 3:
                            if (currentUser.Role == "Admin" || currentUser.Role == "Manager")
                                ManageSuppliers();
                            else
                                Console.WriteLine("Access Denied. Manager or Admin role required.");
                            break;
                        case 4:
                            if (currentUser.Role == "Admin" || currentUser.Role == "Manager")
                                ManageReports();
                            else
                                Console.WriteLine("Access Denied. Manager or Admin role required.");
                            break;
                        case 5:
                            if (currentUser.Role == "Admin")
                                ManageUsers();
                            else
                                Console.WriteLine("Invalid option for your role.");
                            break;
                        case 9:
                            Logout();
                            exit = true; // Will exit the loop and return to Program.Main
                            break;
                        case 0:
                            exit = true; // Will exit the loop and allow Program.Main to save
                            break;
                        default:
                            Console.WriteLine("Invalid choice. Please try again.");
                            break;
                    }
                }
                else
                {
                    Console.WriteLine("Invalid input. Please enter a number.");
                }
            }
        }

        // --- User Management (already provided) ---
        public void ManageUsers()
        {
            if (currentUser == null || currentUser.Role != "Admin")
            {
                Console.WriteLine("Access denied. Admin privileges required.");
                return;
            }

            bool managing = true;
            while (managing)
            {
                Console.WriteLine("\n===== USER MANAGEMENT =====");
                Console.WriteLine("1. List All Users");
                Console.WriteLine("2. Add New User");
                Console.WriteLine("3. Edit User");
                Console.WriteLine("4. Deactivate/Activate User");
                Console.WriteLine("5. View User Activity");
                Console.WriteLine("0. Back to Main Menu");
                Console.Write("\nEnter your choice: ");

                if (int.TryParse(Console.ReadLine(), out int choice))
                {
                    switch (choice)
                    {
                        case 0:
                            managing = false;
                            break;
                        case 1:
                            ListUsers();
                            break;
                        case 2:
                            AddUser();
                            break;
                        case 3:
                            EditUser();
                            break;
                        case 4:
                            DeactivateUser();
                            break;
                        case 5:
                            ViewUserActivity();
                            break;
                        default:
                            Console.WriteLine("Invalid option.");
                            break;
                    }
                    // Save data after potentially making changes
                    if (choice > 0 && choice <= 4) SaveData();
                }
                else { Console.WriteLine("Invalid input."); }
            }
        }

        private void ListUsers()
        {
            Console.WriteLine("\n===== USER LIST =====");
            if (!users.Any())
            {
                Console.WriteLine("No users found.");
                return;
            }
            foreach (var user in users)
            {
                Console.WriteLine($"Username: {user.Username} | Role: {user.Role} | Last Login: {user.LastLogin} | Status: {(user.IsActive ? "Active" : "Inactive")}");
            }
        }

        private void AddUser()
        {
            Console.WriteLine("\n===== ADD NEW USER =====");
            Console.Write("Username: ");
            string username = Console.ReadLine();

            if (string.IsNullOrWhiteSpace(username))
            {
                Console.WriteLine("Username cannot be empty.");
                return;
            }

            if (users.Any(u => u.Username.Equals(username, StringComparison.OrdinalIgnoreCase)))
            {
                Console.WriteLine("Username already exists.");
                return;
            }

            Console.Write("Password: ");
            string password = Console.ReadLine();
            if (string.IsNullOrWhiteSpace(password))
            {
                Console.WriteLine("Password cannot be empty.");
                return;
            }

            Console.WriteLine("Available Roles:");
            Console.WriteLine("1. Admin");
            Console.WriteLine("2. Manager");
            Console.WriteLine("3. Staff");
            Console.Write("Select Role (1-3) [Default: Staff]: ");

            string role = "Staff"; // Default
            if (int.TryParse(Console.ReadLine(), out int roleChoice))
            {
                role = roleChoice switch
                {
                    1 => "Admin",
                    2 => "Manager",
                    3 => "Staff",
                    _ => "Staff"
                };
            }

            users.Add(new User(username, password, role));
            currentUser.LogActivity("User Management", $"Added new user: {username} with role: {role}");
            Console.WriteLine($"User {username} added successfully with role: {role}");
        }

        private void EditUser()
        {
            Console.WriteLine("\n===== EDIT USER =====");
            Console.Write("Enter username to edit: ");
            string username = Console.ReadLine();

            var user = users.FirstOrDefault(u => u.Username.Equals(username, StringComparison.OrdinalIgnoreCase));
            if (user == null)
            {
                Console.WriteLine("User not found.");
                return;
            }

            Console.WriteLine($"Editing user: {user.Username} | Current Role: {user.Role} | Status: {(user.IsActive ? "Active" : "Inactive")}");

            Console.Write("Change Password? (Y/N): ");
            if (Console.ReadLine()?.Trim().ToUpper() == "Y")
            {
                Console.Write("New Password: ");
                string newPassword = Console.ReadLine();
                if (!string.IsNullOrWhiteSpace(newPassword))
                {
                    user.PasswordHash = User.HashPassword(newPassword);
                    currentUser.LogActivity("User Management", $"Changed password for user: {user.Username}");
                    Console.WriteLine("Password updated.");
                }
                else
                {
                    Console.WriteLine("Password not changed (cannot be empty).");
                }
            }

            Console.Write("Change Role? (Y/N): ");
            if (Console.ReadLine()?.Trim().ToUpper() == "Y")
            {
                Console.WriteLine("Available Roles:");
                Console.WriteLine("1. Admin");
                Console.WriteLine("2. Manager");
                Console.WriteLine("3. Staff");
                Console.Write("Select New Role (1-3): ");

                if (int.TryParse(Console.ReadLine(), out int roleChoice))
                {
                    string newRole = roleChoice switch
                    {
                        1 => "Admin",
                        2 => "Manager",
                        3 => "Staff",
                        _ => user.Role // Keep current role if invalid input
                    };

                    if (newRole != user.Role)
                    {
                        // Prevent demoting the only admin
                        if (user.Role == "Admin" && users.Count(u => u.Role == "Admin" && u.IsActive) <= 1)
                        {
                            Console.WriteLine("Cannot change role. At least one active Admin must exist.");
                        }
                        else
                        {
                            currentUser.LogActivity("User Management", $"Changed role for {user.Username} from {user.Role} to {newRole}");
                            user.Role = newRole;
                            Console.WriteLine($"Role updated to {newRole}.");
                        }
                    }
                }
                else
                {
                    Console.WriteLine("Invalid role choice. Role not changed.");
                }
            }

            Console.Write($"{(user.IsActive ? "Deactivate" : "Activate")} User? (Y/N): ");
            if (Console.ReadLine()?.Trim().ToUpper() == "Y")
            {
                DeactivateUser(user.Username); // Reuse the deactivate logic
            }

        }

        private void DeactivateUser(string usernameToToggle = null)
        {
            string username;
            if (usernameToToggle == null)
            {
                Console.WriteLine("\n===== DEACTIVATE/ACTIVATE USER =====");
                Console.Write("Enter username to deactivate/activate: ");
                username = Console.ReadLine();
            }
            else
            {
                username = usernameToToggle;
            }

            var user = users.FirstOrDefault(u => u.Username.Equals(username, StringComparison.OrdinalIgnoreCase));
            if (user == null)
            {
                Console.WriteLine("User not found.");
                return;
            }

            if (user.Username.Equals(currentUser.Username, StringComparison.OrdinalIgnoreCase))
            {
                Console.WriteLine("You cannot deactivate your own account.");
                return;
            }

            // Prevent deactivating the only admin
            if (user.Role == "Admin" && !user.IsActive == false && users.Count(u => u.Role == "Admin" && u.IsActive) <= 1)
            {
                Console.WriteLine("Cannot deactivate the last active Admin user.");
                return;
            }

            user.IsActive = !user.IsActive;
            string status = user.IsActive ? "activated" : "deactivated";
            currentUser.LogActivity("User Management", $"{status} user: {username}");
            Console.WriteLine($"User {username} has been {status}.");
        }

        private void ViewUserActivity()
        {
            Console.WriteLine("\n===== USER ACTIVITY LOG =====");
            Console.Write("Enter username (leave blank for all users): ");
            string username = Console.ReadLine();

            List<User> usersToShow;
            if (string.IsNullOrWhiteSpace(username))
            {
                usersToShow = users;
            }
            else
            {
                var user = users.FirstOrDefault(u => u.Username.Equals(username, StringComparison.OrdinalIgnoreCase));
                if (user == null)
                {
                    Console.WriteLine("User not found.");
                    return;
                }
                usersToShow = new List<User> { user };
            }

            foreach (var user in usersToShow)
            {
                Console.WriteLine($"\n--- Activity for {user.Username} ---");
                if (user.ActivityLog == null || user.ActivityLog.Count == 0)
                {
                    Console.WriteLine(" No activity recorded.");
                    continue;
                }

                // Show latest 20 activities per user, newest first
                foreach (var activity in user.ActivityLog.OrderByDescending(a => a.Timestamp).Take(20))
                {
                    Console.WriteLine($" {activity}");
                }
                if (user.ActivityLog.Count > 20)
                {
                    Console.WriteLine($" ... (older activities not shown)");
                }
            }
        }

        // --- Supplier Management ---
        public void ManageSuppliers()
        {
            if (currentUser == null || (currentUser.Role != "Admin" && currentUser.Role != "Manager"))
            {
                Console.WriteLine("Access denied. Manager or Admin role required.");
                return;
            }

            bool managing = true;
            while (managing)
            {
                Console.WriteLine("\n===== SUPPLIER & PURCHASING =====");
                Console.WriteLine("--- Suppliers ---");
                Console.WriteLine("1. List All Suppliers");
                Console.WriteLine("2. Add New Supplier");
                Console.WriteLine("3. Edit Supplier");
                Console.WriteLine("4. View Supplier Details");
                Console.WriteLine("--- Purchase Orders ---");
                Console.WriteLine("5. Create Purchase Order");
                Console.WriteLine("6. List Purchase Orders");
                Console.WriteLine("7. Update Purchase Order Status / Receive Stock");
                Console.WriteLine("0. Back to Main Menu");
                Console.Write("\nEnter your choice: ");

                if (int.TryParse(Console.ReadLine(), out int choice))
                {
                    switch (choice)
                    {
                        case 0:
                            managing = false;
                            break;
                        case 1:
                            ListSuppliers();
                            break;
                        case 2:
                            AddSupplier();
                            break;
                        case 3:
                            EditSupplier();
                            break;
                        case 4:
                            ViewSupplierDetails();
                            break;
                        case 5:
                            CreatePurchaseOrder();
                            break;
                        case 6:
                            ListPurchaseOrders();
                            break;
                        case 7:
                            UpdatePurchaseOrderStatus();
                            break;
                        default:
                            Console.WriteLine("Invalid option.");
                            break;
                    }
                    // Save data after potentially making changes
                    if (choice > 0 && choice <= 7) SaveData();
                }
                else { Console.WriteLine("Invalid input."); }
            }
        }

        private void ListSuppliers()
        {
            Console.WriteLine("\n===== SUPPLIER LIST =====");
            if (suppliers.Count == 0)
            {
                Console.WriteLine("No suppliers found.");
                return;
            }

            foreach (var supplier in suppliers.OrderBy(s => s.Name))
            {
                Console.WriteLine(supplier);
            }
        }

        private void AddSupplier()
        {
            Console.WriteLine("\n===== ADD NEW SUPPLIER =====");
            var supplier = new Supplier();

            Console.Write("Supplier Name: ");
            supplier.Name = Console.ReadLine();
            if (string.IsNullOrWhiteSpace(supplier.Name)) { Console.WriteLine("Supplier name cannot be empty."); return; }

            Console.Write("Contact Person: ");
            supplier.ContactPerson = Console.ReadLine();

            Console.Write("Email: ");
            supplier.Email = Console.ReadLine();

            Console.Write("Phone: ");
            supplier.Phone = Console.ReadLine();

            Console.Write("Address: ");
            supplier.Address = Console.ReadLine();

            suppliers.Add(supplier);
            currentUser.LogActivity("Supplier Management", $"Added new supplier: {supplier.Name} (ID: {supplier.Id})");
            Console.WriteLine($"Supplier added successfully. Supplier ID: {supplier.Id}");
        }

        private void EditSupplier()
        {
            Console.WriteLine("\n===== EDIT SUPPLIER =====");
            Console.Write("Enter Supplier ID to edit: ");
            string id = Console.ReadLine()?.Trim();

            var supplier = suppliers.FirstOrDefault(s => s.Id.Equals(id, StringComparison.OrdinalIgnoreCase));
            if (supplier == null)
            {
                Console.WriteLine("Supplier not found.");
                return;
            }

            Console.WriteLine("\nCurrent Supplier Details:");
            Console.WriteLine(supplier);
            Console.WriteLine($"Contact Person: {supplier.ContactPerson}");
            Console.WriteLine($"Email: {supplier.Email}");
            Console.WriteLine($"Phone: {supplier.Phone}");
            Console.WriteLine($"Address: {supplier.Address}");
            Console.WriteLine($"Status: {(supplier.IsActive ? "Active" : "Inactive")}");

            Console.WriteLine("\nEnter new details (leave blank to keep current value):");

            Console.Write($"Name [{supplier.Name}]: ");
            string input = Console.ReadLine();
            if (!string.IsNullOrWhiteSpace(input)) supplier.Name = input;

            Console.Write($"Contact Person [{supplier.ContactPerson}]: ");
            input = Console.ReadLine();
            if (!string.IsNullOrWhiteSpace(input)) supplier.ContactPerson = input;

            Console.Write($"Email [{supplier.Email}]: ");
            input = Console.ReadLine();
            if (!string.IsNullOrWhiteSpace(input)) supplier.Email = input;

            Console.Write($"Phone [{supplier.Phone}]: ");
            input = Console.ReadLine();
            if (!string.IsNullOrWhiteSpace(input)) supplier.Phone = input;

            Console.Write($"Address [{supplier.Address}]: ");
            input = Console.ReadLine();
            if (!string.IsNullOrWhiteSpace(input)) supplier.Address = input;

            Console.Write($"Active (Y/N) [{(supplier.IsActive ? "Y" : "N")}]: ");
            input = Console.ReadLine()?.ToUpper();
            if (input == "Y") supplier.IsActive = true;
            else if (input == "N") supplier.IsActive = false;

            currentUser.LogActivity("Supplier Management", $"Updated supplier: {supplier.Name} (ID: {supplier.Id})");
            Console.WriteLine("Supplier updated successfully.");
        }

        private void ViewSupplierDetails()
        {
            Console.WriteLine("\n===== VIEW SUPPLIER DETAILS =====");
            Console.Write("Enter Supplier ID: ");
            string id = Console.ReadLine()?.Trim();

            var supplier = suppliers.FirstOrDefault(s => s.Id.Equals(id, StringComparison.OrdinalIgnoreCase));
            if (supplier == null)
            {
                Console.WriteLine("Supplier not found.");
                return;
            }

            Console.WriteLine("\n--- Supplier Information ---");
            Console.WriteLine($"ID: {supplier.Id}");
            Console.WriteLine($"Name: {supplier.Name}");
            Console.WriteLine($"Contact Person: {supplier.ContactPerson}");
            Console.WriteLine($"Email: {supplier.Email}");
            Console.WriteLine($"Phone: {supplier.Phone}");
            Console.WriteLine($"Address: {supplier.Address}");
            Console.WriteLine($"Status: {(supplier.IsActive ? "Active" : "Inactive")}");
            Console.WriteLine($"Last Order Date: {(supplier.LastOrderDate == DateTime.MinValue ? "N/A" : supplier.LastOrderDate.ToShortDateString())}");
            Console.WriteLine($"Total Orders Placed: {supplier.TotalOrders}");
            Console.WriteLine($"Late Deliveries Recorded: {supplier.LateDeliveries}");
            Console.WriteLine($"Average Lead Time: {supplier.AverageLeadTimeDays:F1} days");

            Console.WriteLine("\n--- Recent Purchase Orders from this Supplier ---");
            var relatedPOs = purchaseOrders
                .Where(po => po.SupplierId == supplier.Id)
                .OrderByDescending(po => po.OrderDate)
                .Take(10) // Limit to recent 10
                .ToList();

            if (!relatedPOs.Any())
            {
                Console.WriteLine("No purchase orders found for this supplier.");
            }
            else
            {
                foreach (var po in relatedPOs)
                {
                    Console.WriteLine(po);
                }
                if (purchaseOrders.Count(po => po.SupplierId == supplier.Id) > 10)
                {
                    Console.WriteLine("... (older orders not shown)");
                }
            }
        }

        private void CreatePurchaseOrder()
        {
            Console.WriteLine("\n===== CREATE PURCHASE ORDER =====");
            if (!suppliers.Any(s => s.IsActive))
            {
                Console.WriteLine("No active suppliers available. Please add or activate a supplier first.");
                return;
            }

            Console.WriteLine("Available Active Suppliers:");
            ListSuppliers(); // Show list to help user choose
            Console.Write("Enter Supplier ID for the PO: ");
            string supplierId = Console.ReadLine()?.Trim();

            var supplier = suppliers.FirstOrDefault(s => s.Id.Equals(supplierId, StringComparison.OrdinalIgnoreCase) && s.IsActive);
            if (supplier == null)
            {
                Console.WriteLine("Active supplier not found with that ID.");
                return;
            }

            var po = new PurchaseOrder
            {
                SupplierId = supplier.Id,
                SupplierName = supplier.Name, // Store name at time of order
                Status = "Draft" // Initial status
            };

            Console.WriteLine($"Creating PO for Supplier: {supplier.Name}");

            // Add items to PO
            bool addMoreItems = true;
            while (addMoreItems)
            {
                Console.WriteLine("\n--- Add Item to Purchase Order ---");
                Console.Write("Enter Product ID (or part of Name to search): ");
                string search = Console.ReadLine()?.Trim();
                if (string.IsNullOrWhiteSpace(search)) break; // Finish adding items

                var foundProducts = products.Where(p => p.Id.Equals(search, StringComparison.OrdinalIgnoreCase) || p.Name.Contains(search, StringComparison.OrdinalIgnoreCase)).ToList();

                Product selectedProduct = null;
                if (foundProducts.Count == 0)
                {
                    Console.WriteLine("No product found matching that ID or name.");
                    continue; // Ask for product again
                }
                else if (foundProducts.Count == 1)
                {
                    selectedProduct = foundProducts[0];
                    Console.WriteLine($"Selected Product: {selectedProduct.Name} (ID: {selectedProduct.Id})");
                }
                else
                {
                    Console.WriteLine("Multiple products found:");
                    for (int i = 0; i < foundProducts.Count; i++)
                    {
                        Console.WriteLine($"{i + 1}. {foundProducts[i]}");
                    }
                    Console.Write("Select product number: ");
                    if (int.TryParse(Console.ReadLine(), out int pChoice) && pChoice > 0 && pChoice <= foundProducts.Count)
                    {
                        selectedProduct = foundProducts[pChoice - 1];
                    }
                    else
                    {
                        Console.WriteLine("Invalid selection.");
                        continue;
                    }
                }

                // Check if item already in PO
                if (po.Items.Any(i => i.ProductId == selectedProduct.Id))
                {
                    Console.WriteLine("Product already added to this PO. You can edit quantity later if needed.");
                    continue;
                }

                Console.Write($"Enter Quantity for {selectedProduct.Name}: ");
                if (!int.TryParse(Console.ReadLine(), out int quantity) || quantity <= 0)
                {
                    Console.WriteLine("Invalid quantity. Must be a positive number.");
                    continue;
                }

                Console.Write($"Enter Cost Price per unit for this PO [{selectedProduct.CostPrice:F2}]: $");
                if (!decimal.TryParse(Console.ReadLine(), NumberStyles.Currency, CultureInfo.InvariantCulture, out decimal costPrice) || costPrice < 0)
                {
                    // Use default cost price from product if input is invalid or blank
                    costPrice = selectedProduct.CostPrice;
                    Console.WriteLine($"Using default cost price: ${costPrice:F2}");
                }

                po.Items.Add(new PurchaseOrderItem
                {
                    ProductId = selectedProduct.Id,
                    ProductName = selectedProduct.Name, // Store name at time of order
                    CostPrice = costPrice,
                    Quantity = quantity
                });

                Console.WriteLine("Item added to PO.");

                Console.Write("Add another item? (Y/N): ");
                addMoreItems = Console.ReadLine()?.Trim().ToUpper() == "Y";
            }

            if (po.Items.Count == 0)
            {
                Console.WriteLine("Purchase Order cancelled as no items were added.");
                return;
            }

            // Optional: Expected Delivery Date
            Console.Write("Enter Expected Delivery Date (YYYY-MM-DD, optional): ");
            string dateInput = Console.ReadLine();
            if (DateTime.TryParseExact(dateInput, "yyyy-MM-dd", CultureInfo.InvariantCulture, DateTimeStyles.None, out DateTime expectedDate))
            {
                po.ExpectedDeliveryDate = expectedDate;
            }

            // Final confirmation? (Optional)
            Console.WriteLine("\n--- Purchase Order Summary ---");
            Console.WriteLine($"PO ID: {po.PoId}");
            Console.WriteLine($"Supplier: {po.SupplierName}");
            Console.WriteLine($"Order Date: {po.OrderDate.ToShortDateString()}");
            Console.WriteLine($"Expected Delivery: {po.ExpectedDeliveryDate?.ToShortDateString() ?? "N/A"}");
            Console.WriteLine("Items:");
            foreach (var item in po.Items)
            {
                Console.WriteLine($" - {item.ProductName} (ID: {item.ProductId}), Qty: {item.Quantity}, Cost: ${item.CostPrice:F2}");
            }
            Console.WriteLine($"Total Cost: ${po.GetTotal():F2}");

            Console.Write("\nSave this Purchase Order as Draft? (Y/N): ");
            if (Console.ReadLine()?.Trim().ToUpper() == "Y")
            {
                purchaseOrders.Add(po);
                currentUser.LogActivity("Purchase Order", $"Created Draft PO: {po.PoId} for supplier {po.SupplierName}");
                Console.WriteLine($"Purchase Order {po.PoId} created successfully as Draft.");
            }
            else
            {
                Console.WriteLine("Purchase Order creation cancelled.");
            }
        }

        private void ListPurchaseOrders()
        {
            Console.WriteLine("\n===== PURCHASE ORDER LIST =====");
            if (!purchaseOrders.Any())
            {
                Console.WriteLine("No purchase orders found.");
                return;
            }

            Console.WriteLine("Filter by Status? (Leave blank for all, or enter Draft, Submitted, Received, Cancelled): ");
            string statusFilter = Console.ReadLine()?.Trim();

            var filteredPOs = purchaseOrders;
            if (!string.IsNullOrWhiteSpace(statusFilter))
            {
                filteredPOs = purchaseOrders.Where(po => po.Status.Equals(statusFilter, StringComparison.OrdinalIgnoreCase)).ToList();
            }

            if (!filteredPOs.Any())
            {
                Console.WriteLine($"No purchase orders found with status '{statusFilter}'.");
                return;
            }

            Console.WriteLine("\n--- Purchase Orders ---");
            foreach (var po in filteredPOs.OrderByDescending(p => p.OrderDate))
            {
                Console.WriteLine(po);
            }
        }

        private void UpdatePurchaseOrderStatus()
        {
            Console.WriteLine("\n===== UPDATE PURCHASE ORDER STATUS / RECEIVE STOCK =====");
            Console.Write("Enter Purchase Order ID (PO-XXXXXX): ");
            string poId = Console.ReadLine()?.Trim();

            var po = purchaseOrders.FirstOrDefault(p => p.PoId.Equals(poId, StringComparison.OrdinalIgnoreCase));
            if (po == null)
            {
                Console.WriteLine("Purchase Order not found.");
                return;
            }

            Console.WriteLine($"\nCurrent PO Details: {po}");
            Console.WriteLine("Current Status: " + po.Status);

            Console.WriteLine("\nAvailable Actions:");
            Console.WriteLine("1. Mark as Submitted");
            Console.WriteLine("2. Mark as Received (and update stock)");
            Console.WriteLine("3. Mark as Cancelled");
            Console.WriteLine("0. Go Back");
            Console.Write("Select action: ");

            if (!int.TryParse(Console.ReadLine(), out int choice))
            {
                Console.WriteLine("Invalid choice.");
                return;
            }

            string oldStatus = po.Status;
            string newStatus = po.Status; // Default to current
            bool statusChanged = false;

            switch (choice)
            {
                case 1: // Submitted
                    if (po.Status == "Draft")
                    {
                        newStatus = "Submitted";
                        po.Status = newStatus;
                        // Update supplier metrics - Order Placed
                        var supplier = suppliers.FirstOrDefault(s => s.Id == po.SupplierId);
                        if (supplier != null)
                        {
                            supplier.TotalOrders++;
                            supplier.LastOrderDate = po.OrderDate;
                        }
                        currentUser.LogActivity("Purchase Order", $"Marked PO {po.PoId} as Submitted.");
                        Console.WriteLine("PO status updated to Submitted.");
                        statusChanged = true;
                    }
                    else
                    {
                        Console.WriteLine($"Cannot mark as Submitted. Current status is {po.Status}.");
                    }
                    break;

                case 2: // Received
                    if (po.Status == "Submitted")
                    {
                        Console.Write("Enter Actual Delivery Date (YYYY-MM-DD) [Default: Today]: ");
                        string dateInput = Console.ReadLine();
                        if (!DateTime.TryParseExact(dateInput, "yyyy-MM-dd", CultureInfo.InvariantCulture, DateTimeStyles.None, out DateTime actualDeliveryDate))
                        {
                            actualDeliveryDate = DateTime.Today; // Use today if input is invalid/blank
                        }
                        po.ActualDeliveryDate = actualDeliveryDate;

                        Console.WriteLine("\nUpdating stock levels...");
                        bool stockUpdateError = false;
                        foreach (var item in po.Items)
                        {
                            var product = products.FirstOrDefault(p => p.Id == item.ProductId);
                            if (product != null)
                            {
                                product.Quantity += item.Quantity;
                                product.LastRestocked = actualDeliveryDate;
                                // Optionally update product cost price based on this PO
                                // Example: Update to latest cost
                                product.CostPrice = item.CostPrice;
                                Console.WriteLine($" + {item.Quantity} units added to {product.Name} (New Stock: {product.Quantity}). Cost Price updated to ${item.CostPrice:F2}.");

                                // Associate product with supplier if not already done (or update supplier)
                                product.SupplierId = po.SupplierId;
                                product.SupplierName = po.SupplierName;

                            }
                            else
                            {
                                Console.WriteLine($" WARNING: Product ID {item.ProductId} ({item.ProductName}) not found in inventory. Stock not updated.");
                                stockUpdateError = true;
                            }
                        }

                        if (stockUpdateError)
                        {
                            Console.WriteLine("Warning: Some products in the PO were not found in the main inventory. Stock for those items was not updated.");
                        }

                        // Update supplier performance metrics
                        var receivingSupplier = suppliers.FirstOrDefault(s => s.Id == po.SupplierId);
                        if (receivingSupplier != null)
                        {
                            TimeSpan leadTime = actualDeliveryDate - po.OrderDate;
                            // Basic average lead time calculation (can be improved)
                            receivingSupplier.AverageLeadTimeDays = (receivingSupplier.AverageLeadTimeDays * (receivingSupplier.TotalOrders - 1) + (decimal)leadTime.TotalDays) / receivingSupplier.TotalOrders;

                            if (po.ExpectedDeliveryDate.HasValue && actualDeliveryDate > po.ExpectedDeliveryDate.Value)
                            {
                                receivingSupplier.LateDeliveries++;
                                Console.WriteLine("Delivery was late based on expected date.");
                            }
                        }

                        newStatus = "Received";
                        po.Status = newStatus;
                        currentUser.LogActivity("Purchase Order", $"Marked PO {po.PoId} as Received. Stock updated.");
                        Console.WriteLine($"PO status updated to Received. Stock levels adjusted.");
                        statusChanged = true;
                    }
                    else
                    {
                        Console.WriteLine($"Cannot mark as Received. Current status is {po.Status}. (Must be Submitted first).");
                    }
                    break;

                case 3: // Cancelled
                    if (po.Status == "Draft" || po.Status == "Submitted")
                    {
                        // If cancelled after submission, potentially adjust supplier metrics (e.g., remove from TotalOrders if it was added at submission)
                        if (po.Status == "Submitted")
                        {
                            var supplierToAdjust = suppliers.FirstOrDefault(s => s.Id == po.SupplierId);
                            if (supplierToAdjust != null && supplierToAdjust.TotalOrders > 0)
                            {
                                // Decrement order count if it was incremented at submission
                                supplierToAdjust.TotalOrders--;
                                // Recalculating average lead time on cancellation is complex, might skip or adjust based on policy
                            }
                        }
                        newStatus = "Cancelled";
                        po.Status = newStatus;
                        currentUser.LogActivity("Purchase Order", $"Cancelled PO {po.PoId}.");
                        Console.WriteLine("PO status updated to Cancelled.");
                        statusChanged = true;
                    }
                    else
                    {
                        Console.WriteLine($"Cannot cancel PO. Current status is {po.Status}. (Cannot cancel Received orders).");
                    }
                    break;

                case 0: // Go Back
                    return;

                default:
                    Console.WriteLine("Invalid action choice.");
                    break;
            }

            // Log the status change if it happened
            if (statusChanged)
            {
                currentUser.LogActivity("PO Status Update", $"Changed status for PO {po.PoId} from {oldStatus} to {newStatus}");
            }
        }

        // --- Product Management ---
        public void ManageProducts()
        {
            // Allow Staff role to view/search but not modify
            bool canModify = (currentUser.Role == "Admin" || currentUser.Role == "Manager");

            bool managing = true;
            while (managing)
            {
                Console.WriteLine("\n===== PRODUCT MANAGEMENT =====");
                Console.WriteLine("1. List All Products");
                Console.WriteLine("2. Search Products");
                if (canModify) Console.WriteLine("3. Add New Product");
                if (canModify) Console.WriteLine("4. Edit Product");
                if (canModify) Console.WriteLine("5. Activate/Deactivate Product");
                Console.WriteLine("0. Back to Main Menu");
                Console.Write("\nEnter your choice: ");

                if (int.TryParse(Console.ReadLine(), out int choice))
                {
                    switch (choice)
                    {
                        case 0:
                            managing = false;
                            break;
                        case 1:
                            ListProducts();
                            break;
                        case 2:
                            SearchProducts();
                            break;
                        case 3:
                            if (canModify) AddProduct(); else Console.WriteLine("Access Denied.");
                            break;
                        case 4:
                            if (canModify) EditProduct(); else Console.WriteLine("Access Denied.");
                            break;
                        case 5:
                            if (canModify) ToggleProductActivation(); else Console.WriteLine("Access Denied.");
                            break;
                        default:
                            Console.WriteLine("Invalid option.");
                            break;
                    }
                    // Save data after potentially making changes
                    if (canModify && choice >= 3 && choice <= 5) SaveData();
                }
                else { Console.WriteLine("Invalid input."); }
            }
        }

        private void ListProducts()
        {
            Console.WriteLine("\n===== PRODUCT LIST =====");
            if (!products.Any())
            {
                Console.WriteLine("No products found in inventory.");
                return;
            }

            Console.WriteLine("Sort by: (1) Name, (2) ID, (3) Category, (4) Quantity [Default: Name]");
            Console.Write("Enter sort choice: ");
            int.TryParse(Console.ReadLine(), out int sortChoice);

            var sortedProducts = products.AsEnumerable(); // Start with IEnumerable for sorting

            switch (sortChoice)
            {
                case 2: sortedProducts = sortedProducts.OrderBy(p => p.Id); break;
                case 3: sortedProducts = sortedProducts.OrderBy(p => p.Category).ThenBy(p => p.Name); break;
                case 4: sortedProducts = sortedProducts.OrderBy(p => p.Quantity).ThenBy(p => p.Name); break;
                case 1:
                default: sortedProducts = sortedProducts.OrderBy(p => p.Name); break;
            }

            foreach (var product in sortedProducts)
            {
                Console.WriteLine(product);
            }
            Console.WriteLine($"\nTotal different product lines: {products.Count}");
            Console.WriteLine($"Total stock units: {products.Sum(p => p.Quantity)}");
        }

        private void SearchProducts()
        {
            Console.WriteLine("\n===== SEARCH PRODUCTS =====");
            Console.Write("Enter search term (ID, Name, Category, Supplier Name): ");
            string searchTerm = Console.ReadLine()?.Trim().ToLower();

            if (string.IsNullOrWhiteSpace(searchTerm))
            {
                Console.WriteLine("Search term cannot be empty.");
                ListProducts(); // Show all if search is empty
                return;
            }

            var results = products.Where(p =>
                p.Id.Equals(searchTerm, StringComparison.OrdinalIgnoreCase) ||
                p.Name.ToLower().Contains(searchTerm) ||
                p.Category.ToLower().Contains(searchTerm) ||
                (p.SupplierName != null && p.SupplierName.ToLower().Contains(searchTerm))
            ).OrderBy(p => p.Name).ToList();

            if (!results.Any())
            {
                Console.WriteLine("No products found matching your search criteria.");
            }
            else
            {
                Console.WriteLine($"\n--- Search Results ({results.Count} found) ---");
                foreach (var product in results)
                {
                    Console.WriteLine(product);
                }
            }
        }

        private void AddProduct()
        {
            Console.WriteLine("\n===== ADD NEW PRODUCT =====");
            var product = new Product();

            Console.Write("Product Name: ");
            product.Name = Console.ReadLine();
            if (string.IsNullOrWhiteSpace(product.Name)) { Console.WriteLine("Product name cannot be empty."); return; }

            Console.Write("Description: ");
            product.Description = Console.ReadLine();

            Console.Write("Category: ");
            product.Category = Console.ReadLine();
            if (string.IsNullOrWhiteSpace(product.Category)) { Console.WriteLine("Category cannot be empty."); return; }

            Console.Write("Selling Price ($): ");
            if (decimal.TryParse(Console.ReadLine(), NumberStyles.Currency, CultureInfo.InvariantCulture, out decimal price) && price >= 0)
                product.Price = price;
            else { Console.WriteLine("Invalid selling price. Aborting."); return; }

            Console.Write("Cost Price ($): ");
            if (decimal.TryParse(Console.ReadLine(), NumberStyles.Currency, CultureInfo.InvariantCulture, out decimal costPrice) && costPrice >= 0)
                product.CostPrice = costPrice;
            else { Console.WriteLine("Invalid cost price. Setting to 0."); product.CostPrice = 0; }

            Console.Write("Initial Quantity: ");
            if (int.TryParse(Console.ReadLine(), out int quantity) && quantity >= 0)
                product.Quantity = quantity;
            else { Console.WriteLine("Invalid quantity. Setting to 0."); product.Quantity = 0; }

            // Assign Supplier
            Console.WriteLine("\nAvailable Active Suppliers:");
            var activeSuppliers = suppliers.Where(s => s.IsActive).ToList();
            if (!activeSuppliers.Any())
            {
                Console.WriteLine("No active suppliers found. You can add supplier info later by editing the product.");
            }
            else
            {
                for (int i = 0; i < activeSuppliers.Count; i++)
                {
                    Console.WriteLine($"{i + 1}. {activeSuppliers[i].Name} (ID: {activeSuppliers[i].Id})");
                }
                Console.Write("Select Supplier number (or leave blank if none/unknown): ");
                if (int.TryParse(Console.ReadLine(), out int supChoice) && supChoice > 0 && supChoice <= activeSuppliers.Count)
                {
                    var selectedSupplier = activeSuppliers[supChoice - 1];
                    product.SupplierId = selectedSupplier.Id;
                    product.SupplierName = selectedSupplier.Name;
                    Console.WriteLine($"Supplier set to: {selectedSupplier.Name}");
                }
                else
                {
                    Console.WriteLine("No supplier selected for this product.");
                }
            }

            Console.Write($"Default Tax Rate (%) [{product.TaxRate * 100}]: ");
            if (decimal.TryParse(Console.ReadLine(), out decimal taxRate) && taxRate >= 0 && taxRate <= 100)
                product.TaxRate = taxRate / 100m;
            else { Console.WriteLine($"Using default tax rate: {product.TaxRate * 100}%"); }

            Console.Write($"Default Discount (%) [{product.DiscountPercent}]: ");
            if (decimal.TryParse(Console.ReadLine(), out decimal discount) && discount >= 0 && discount <= 100)
                product.DiscountPercent = discount;
            else { Console.WriteLine($"Using default discount: {product.DiscountPercent}%"); }

            product.IsActive = true; // New products are active by default
            product.LastRestocked = DateTime.Now; // Or set based on initial quantity source?

            products.Add(product);
            currentUser.LogActivity("Product Management", $"Added new product: {product.Name} (ID: {product.Id})");
            Console.WriteLine($"Product '{product.Name}' added successfully with ID: {product.Id}");
        }

        private void EditProduct()
        {
            Console.WriteLine("\n===== EDIT PRODUCT =====");
            Console.Write("Enter Product ID to edit: ");
            string id = Console.ReadLine()?.Trim();

            var product = products.FirstOrDefault(p => p.Id.Equals(id, StringComparison.OrdinalIgnoreCase));
            if (product == null)
            {
                Console.WriteLine("Product not found.");
                return;
            }

            Console.WriteLine("\nCurrent Product Details:");
            Console.WriteLine(product);
            Console.WriteLine($"Description: {product.Description}");
            Console.WriteLine($"Supplier: {product.SupplierName ?? "N/A"} (ID: {product.SupplierId ?? "N/A"})");
            Console.WriteLine($"Tax Rate: {product.TaxRate * 100}%");
            Console.WriteLine($"Discount: {product.DiscountPercent}%");
            Console.WriteLine($"Last Restocked: {product.LastRestocked.ToShortDateString()}");

            Console.WriteLine("\nEnter new details (leave blank to keep current value):");

            Console.Write($"Name [{product.Name}]: ");
            string input = Console.ReadLine();
            if (!string.IsNullOrWhiteSpace(input)) product.Name = input;

            Console.Write($"Description [{product.Description}]: ");
            input = Console.ReadLine();
            if (!string.IsNullOrWhiteSpace(input)) product.Description = input; // Allow clearing description

            Console.Write($"Category [{product.Category}]: ");
            input = Console.ReadLine();
            if (!string.IsNullOrWhiteSpace(input)) product.Category = input;

            Console.Write($"Selling Price [${product.Price:F2}]: $");
            if (decimal.TryParse(Console.ReadLine(), NumberStyles.Currency, CultureInfo.InvariantCulture, out decimal price) && price >= 0)
                product.Price = price;

            Console.Write($"Cost Price [${product.CostPrice:F2}]: $");
            if (decimal.TryParse(Console.ReadLine(), NumberStyles.Currency, CultureInfo.InvariantCulture, out decimal costPrice) && costPrice >= 0)
                product.CostPrice = costPrice;

            // Note: Editing quantity directly here is usually discouraged.
            // Quantity should change via Orders or Purchase Order receiving.
            // We *could* add an "Adjust Stock" feature separately if needed.
            Console.WriteLine($"Current Quantity: {product.Quantity} (Use PO Receiving or Order process to change stock)");

            // Edit Supplier Association
            Console.Write($"Change Supplier? (Current: {product.SupplierName ?? "N/A"}) (Y/N): ");
            if (Console.ReadLine()?.Trim().ToUpper() == "Y")
            {
                Console.WriteLine("\nAvailable Active Suppliers:");
                var activeSuppliers = suppliers.Where(s => s.IsActive).ToList();
                if (!activeSuppliers.Any())
                {
                    Console.WriteLine("No active suppliers found.");
                }
                else
                {
                    for (int i = 0; i < activeSuppliers.Count; i++)
                    {
                        Console.WriteLine($"{i + 1}. {activeSuppliers[i].Name} (ID: {activeSuppliers[i].Id})");
                    }
                    Console.Write("Select Supplier number (or 0 to remove supplier): ");
                    if (int.TryParse(Console.ReadLine(), out int supChoice))
                    {
                        if (supChoice == 0)
                        {
                            product.SupplierId = null;
                            product.SupplierName = null;
                            Console.WriteLine("Supplier removed from product.");
                        }
                        else if (supChoice > 0 && supChoice <= activeSuppliers.Count)
                        {
                            var selectedSupplier = activeSuppliers[supChoice - 1];
                            product.SupplierId = selectedSupplier.Id;
                            product.SupplierName = selectedSupplier.Name;
                            Console.WriteLine($"Supplier updated to: {selectedSupplier.Name}");
                        }
                        else
                        {
                            Console.WriteLine("Invalid selection. Supplier not changed.");
                        }
                    }
                    else
                    {
                        Console.WriteLine("Invalid input. Supplier not changed.");
                    }
                }
            }

            Console.Write($"Tax Rate (%) [{product.TaxRate * 100}]: ");
            if (decimal.TryParse(Console.ReadLine(), out decimal taxRate) && taxRate >= 0 && taxRate <= 100)
                product.TaxRate = taxRate / 100m;

            Console.Write($"Discount (%) [{product.DiscountPercent}]: ");
            if (decimal.TryParse(Console.ReadLine(), out decimal discount) && discount >= 0 && discount <= 100)
                product.DiscountPercent = discount;

            // Activation status is handled separately
            Console.WriteLine($"Activation Status: {(product.IsActive ? "Active" : "Inactive")} (Use Activate/Deactivate option to change)");

            currentUser.LogActivity("Product Management", $"Edited product: {product.Name} (ID: {product.Id})");
            Console.WriteLine("Product updated successfully.");
        }

        private void ToggleProductActivation()
        {
            Console.WriteLine("\n===== ACTIVATE/DEACTIVATE PRODUCT =====");
            Console.Write("Enter Product ID to toggle status: ");
            string id = Console.ReadLine()?.Trim();

            var product = products.FirstOrDefault(p => p.Id.Equals(id, StringComparison.OrdinalIgnoreCase));
            if (product == null)
            {
                Console.WriteLine("Product not found.");
                return;
            }

            product.IsActive = !product.IsActive;
            string status = product.IsActive ? "activated" : "deactivated";

            currentUser.LogActivity("Product Management", $"{status} product: {product.Name} (ID: {product.Id})");
            Console.WriteLine($"Product '{product.Name}' has been {status}.");
        }

        // --- Order Management ---
        public void ManageOrders()
        {
            // All roles can potentially interact with orders in some way
            bool managing = true;
            while (managing)
            {
                Console.WriteLine("\n===== ORDER MANAGEMENT =====");
                Console.WriteLine("1. Create New Order");
                Console.WriteLine("2. List Orders");
                Console.WriteLine("3. View Order Details");
                // Status updates might be restricted
                if (currentUser.Role == "Admin" || currentUser.Role == "Manager")
                {
                    Console.WriteLine("4. Update Order Status");
                }
                Console.WriteLine("0. Back to Main Menu");
                Console.Write("\nEnter your choice: ");

                if (int.TryParse(Console.ReadLine(), out int choice))
                {
                    switch (choice)
                    {
                        case 0:
                            managing = false;
                            break;
                        case 1:
                            CreateOrder();
                            break;
                        case 2:
                            ListOrders();
                            break;
                        case 3:
                            ViewOrderDetails();
                            break;
                        case 4:
                            if (currentUser.Role == "Admin" || currentUser.Role == "Manager")
                                UpdateOrderStatus();
                            else
                                Console.WriteLine("Access Denied. Manager or Admin role required for status updates.");
                            break;
                        default:
                            Console.WriteLine("Invalid option.");
                            break;
                    }
                    // Save data after potentially making changes
                    if (choice == 1 || choice == 4) SaveData();
                }
                else { Console.WriteLine("Invalid input."); }
            }
        }

        private void CreateOrder()
        {
            Console.WriteLine("\n===== CREATE NEW CUSTOMER ORDER =====");
            var order = new Order();

            Console.Write("Customer Name: ");
            order.CustomerName = Console.ReadLine();
            if (string.IsNullOrWhiteSpace(order.CustomerName)) { order.CustomerName = "Walk-in Customer"; } // Default name

            Console.WriteLine($"Creating Order for: {order.CustomerName}");
            order.Status = "Processing"; // Initial status

            // Add items to Order
            bool addMoreItems = true;
            while (addMoreItems)
            {
                Console.WriteLine("\n--- Add Item to Order ---");
                Console.Write("Enter Product ID (or part of Name to search): ");
                string search = Console.ReadLine()?.Trim();
                if (string.IsNullOrWhiteSpace(search)) break; // Finish adding items

                // Search only ACTIVE products
                var foundProducts = products.Where(p => p.IsActive && (p.Id.Equals(search, StringComparison.OrdinalIgnoreCase) || p.Name.Contains(search, StringComparison.OrdinalIgnoreCase))).ToList();

                Product selectedProduct = null;
                if (foundProducts.Count == 0)
                {
                    Console.WriteLine("No active product found matching that ID or name.");
                    continue; // Ask for product again
                }
                else if (foundProducts.Count == 1)
                {
                    selectedProduct = foundProducts[0];
                    Console.WriteLine($"Selected Product: {selectedProduct.Name} (ID: {selectedProduct.Id}) | Stock: {selectedProduct.Quantity} | Price: ${selectedProduct.Price:F2}");
                }
                else
                {
                    Console.WriteLine("Multiple active products found:");
                    for (int i = 0; i < foundProducts.Count; i++)
                    {
                        Console.WriteLine($"{i + 1}. {foundProducts[i].Name} (ID: {foundProducts[i].Id}) | Stock: {foundProducts[i].Quantity} | Price: ${foundProducts[i].Price:F2}");
                    }
                    Console.Write("Select product number: ");
                    if (int.TryParse(Console.ReadLine(), out int pChoice) && pChoice > 0 && pChoice <= foundProducts.Count)
                    {
                        selectedProduct = foundProducts[pChoice - 1];
                    }
                    else
                    {
                        Console.WriteLine("Invalid selection.");
                        continue;
                    }
                }

                // Check Stock
                if (selectedProduct.Quantity <= 0)
                {
                    Console.WriteLine($"Product '{selectedProduct.Name}' is out of stock.");
                    continue;
                }

                Console.Write($"Enter Quantity (Available: {selectedProduct.Quantity}): ");
                if (!int.TryParse(Console.ReadLine(), out int quantity) || quantity <= 0)
                {
                    Console.WriteLine("Invalid quantity. Must be a positive number.");
                    continue;
                }

                if (quantity > selectedProduct.Quantity)
                {
                    Console.WriteLine($"Insufficient stock. Only {selectedProduct.Quantity} units available.");
                    continue;
                }

                // Check if item already in the current order being built
                var existingOrderItem = order.Items.FirstOrDefault(i => i.ProductId == selectedProduct.Id);
                if (existingOrderItem != null)
                {
                    // Check if adding more exceeds stock
                    if (existingOrderItem.Quantity + quantity > selectedProduct.Quantity)
                    {
                        Console.WriteLine($"Cannot add {quantity} more. Total requested ({existingOrderItem.Quantity + quantity}) exceeds stock ({selectedProduct.Quantity}).");
                        continue;
                    }
                    existingOrderItem.Quantity += quantity;
                    Console.WriteLine($"Updated quantity for {selectedProduct.Name} to {existingOrderItem.Quantity}.");
                }
                else
                {
                    // Add new item to order
                    order.Items.Add(new OrderItem
                    {
                        ProductId = selectedProduct.Id,
                        ProductName = selectedProduct.Name, // Capture name at time of order
                        Price = selectedProduct.Price, // Capture price at time of order
                        CostPrice = selectedProduct.CostPrice,// Capture cost at time of order
                        Quantity = quantity
                    });
                    Console.WriteLine($"Added {quantity} x {selectedProduct.Name} to the order.");
                }

                Console.Write("Add another item? (Y/N): ");
                addMoreItems = Console.ReadLine()?.Trim().ToUpper() == "Y";
            }

            if (order.Items.Count == 0)
            {
                Console.WriteLine("Order cancelled as no items were added.");
                return;
            }

            // Finalize Order Details
            Console.WriteLine("\n--- Finalize Order ---");
            Console.Write($"Tax Rate (%) [{order.TaxRate * 100}]: ");
            if (decimal.TryParse(Console.ReadLine(), out decimal taxRate) && taxRate >= 0 && taxRate <= 100)
                order.TaxRate = taxRate / 100m;
            else { Console.WriteLine($"Using default tax rate: {order.TaxRate * 100}%"); }

            Console.Write($"Discount Amount ($) [{order.DiscountAmount:F2}]: $");
            if (decimal.TryParse(Console.ReadLine(), NumberStyles.Currency, CultureInfo.InvariantCulture, out decimal discount) && discount >= 0 && discount <= order.GetSubtotal())
                order.DiscountAmount = discount;
            else { Console.WriteLine($"Using default discount: ${order.DiscountAmount:F2}"); }

            Console.Write("Payment Method [Cash]: ");
            string payment = Console.ReadLine();
            if (!string.IsNullOrWhiteSpace(payment)) order.PaymentMethod = payment;

            Console.Write("Mark as Paid? (Y/N) [N]: ");
            order.IsPaid = Console.ReadLine()?.Trim().ToUpper() == "Y";

            // Order Summary
            Console.WriteLine("\n--- Order Summary ---");
            Console.WriteLine($"Order ID: {order.OrderId}");
            Console.WriteLine($"Customer: {order.CustomerName}");
            Console.WriteLine($"Date: {order.OrderDate.ToShortDateString()}");
            Console.WriteLine("Items:");
            foreach (var item in order.Items)
            {
                Console.WriteLine($" - {item.ProductName} (ID: {item.ProductId}), Qty: {item.Quantity}, Price: ${item.Price:F2}");
            }
            Console.WriteLine($"Subtotal: ${order.GetSubtotal():F2}");
            Console.WriteLine($"Tax ({order.TaxRate * 100}%): ${order.GetTaxAmount():F2}");
            Console.WriteLine($"Discount: -${order.DiscountAmount:F2}");
            Console.WriteLine($"Total: ${order.GetTotal():F2}");
            Console.WriteLine($"Payment Method: {order.PaymentMethod}");
            Console.WriteLine($"Paid Status: {(order.IsPaid ? "Paid" : "Unpaid")}");
            Console.WriteLine($"Estimated Profit: ${order.GetProfit():F2}");

            Console.Write("\nConfirm and Create this Order? (Y/N): ");
            if (Console.ReadLine()?.Trim().ToUpper() == "Y")
            {
                // ---- CRITICAL: Update Stock Levels ----
                foreach (var item in order.Items)
                {
                    var product = products.FirstOrDefault(p => p.Id == item.ProductId);
                    if (product != null)
                    {
                        product.Quantity -= item.Quantity;
                        // No need to check stock again here, it was checked during adding
                    }
                    // Log if product somehow disappeared? Unlikely but possible.
                    else
                    {
                        Console.WriteLine($"CRITICAL WARNING: Product {item.ProductId} not found during final stock update for order {order.OrderId}. Inventory might be inconsistent.");
                    }
                }
                // --------------------------------------

                orders.Add(order);
                currentUser.LogActivity("Order Management", $"Created Order: {order.OrderId} for {order.CustomerName}. Total: ${order.GetTotal():F2}");
                Console.WriteLine($"Order {order.OrderId} created successfully.");
            }
            else
            {
                Console.WriteLine("Order creation cancelled. Stock not adjusted.");
            }
        }

        private void ListOrders()
        {
            Console.WriteLine("\n===== ORDER LIST =====");
            if (!orders.Any())
            {
                Console.WriteLine("No customer orders found.");
                return;
            }

            Console.WriteLine("Filter by Status? (Leave blank for all, or enter Pending, Processing, Shipped, Completed, Cancelled): ");
            string statusFilter = Console.ReadLine()?.Trim();

            var filteredOrders = orders.AsEnumerable(); // Use AsEnumerable for potential filtering
            if (!string.IsNullOrWhiteSpace(statusFilter))
            {
                filteredOrders = orders.Where(o => o.Status.Equals(statusFilter, StringComparison.OrdinalIgnoreCase));
            }

            var ordersToShow = filteredOrders.OrderByDescending(o => o.OrderDate).ToList();

            if (!ordersToShow.Any())
            {
                Console.WriteLine(string.IsNullOrWhiteSpace(statusFilter) ? "No orders found." : $"No orders found with status '{statusFilter}'.");
                return;
            }

            Console.WriteLine("\n--- Customer Orders ---");
            foreach (var order in ordersToShow)
            {
                Console.WriteLine(order);
            }
        }

        private void ViewOrderDetails()
        {
            Console.WriteLine("\n===== VIEW ORDER DETAILS =====");
            Console.Write("Enter Order ID (ORD-XXXXXX): ");
            string orderId = Console.ReadLine()?.Trim();

            var order = orders.FirstOrDefault(o => o.OrderId.Equals(orderId, StringComparison.OrdinalIgnoreCase));
            if (order == null)
            {
                Console.WriteLine("Order not found.");
                return;
            }

            Console.WriteLine("\n--- Order Details ---");
            Console.WriteLine($"Order ID: {order.OrderId}");
            Console.WriteLine($"Customer: {order.CustomerName}");
            Console.WriteLine($"Date: {order.OrderDate}");
            Console.WriteLine($"Status: {order.Status}");
            Console.WriteLine("--- Items ---");
            foreach (var item in order.Items)
            {
                Console.WriteLine($" - Product ID: {item.ProductId}");
                Console.WriteLine($" Name: {item.ProductName}");
                Console.WriteLine($" Qty: {item.Quantity}");
                Console.WriteLine($" Unit Price: ${item.Price:F2}");
                Console.WriteLine($" Unit Cost: ${item.CostPrice:F2}");
                Console.WriteLine($" Item Subtotal: ${item.Price * item.Quantity:F2}");
                Console.WriteLine($" Item Profit: ${item.GetProfit():F2}");
            }
            Console.WriteLine("--- Financials ---");
            Console.WriteLine($"Subtotal: ${order.GetSubtotal():F2}");
            Console.WriteLine($"Tax Rate: {order.TaxRate * 100}%");
            Console.WriteLine($"Tax Amount: ${order.GetTaxAmount():F2}");
            Console.WriteLine($"Discount: ${order.DiscountAmount:F2}");
            Console.WriteLine($"Total Amount: ${order.GetTotal():F2}");
            Console.WriteLine($"Total Cost: ${order.GetTotalCost():F2}");
            Console.WriteLine($"Gross Profit: ${order.GetProfit():F2}");
            Console.WriteLine($"Payment Method: {order.PaymentMethod}");
            Console.WriteLine($"Paid Status: {(order.IsPaid ? "Paid" : "Unpaid")}");
        }

        private void UpdateOrderStatus()
        {
            Console.WriteLine("\n===== UPDATE ORDER STATUS =====");
            Console.Write("Enter Order ID (ORD-XXXXXX): ");
            string orderId = Console.ReadLine()?.Trim();

            var order = orders.FirstOrDefault(o => o.OrderId.Equals(orderId, StringComparison.OrdinalIgnoreCase));
            if (order == null)
            {
                Console.WriteLine("Order not found.");
                return;
            }

            Console.WriteLine($"\nCurrent Order Details: {order}");
            Console.WriteLine("Current Status: " + order.Status);

            // Define valid statuses
            var validStatuses = new List<string> { "Pending", "Processing", "Shipped", "Completed", "Cancelled" };

            Console.WriteLine("\nEnter new status (Choose from: Pending, Processing, Shipped, Completed, Cancelled): ");
            string newStatus = Console.ReadLine()?.Trim();

            if (string.IsNullOrWhiteSpace(newStatus) || !validStatuses.Any(s => s.Equals(newStatus, StringComparison.OrdinalIgnoreCase)))
            {
                Console.WriteLine("Invalid status entered.");
                return;
            }
            // Normalize capitalization
            newStatus = validStatuses.First(s => s.Equals(newStatus, StringComparison.OrdinalIgnoreCase));

            if (order.Status.Equals(newStatus, StringComparison.OrdinalIgnoreCase))
            {
                Console.WriteLine("Order status is already set to this value.");
                return;
            }

            // --- Handle Stock Adjustment for Cancellation ---
            if (newStatus.Equals("Cancelled", StringComparison.OrdinalIgnoreCase) && !order.Status.Equals("Cancelled", StringComparison.OrdinalIgnoreCase))
            {
                // Check if order wasn't already completed/shipped (logic might vary based on business rules)
                if (order.Status == "Completed" || order.Status == "Shipped")
                {
                    Console.WriteLine($"Warning: Cancelling an order already marked as '{order.Status}'. Stock was likely already deducted.");
                    Console.Write("Do you still want to proceed and return items to stock? (Y/N): ");
                    if (Console.ReadLine()?.Trim().ToUpper() != "Y")
                    {
                        Console.WriteLine("Cancellation aborted.");
                        return;
                    }
                }

                Console.WriteLine("Returning items to stock...");
                bool stockReturnError = false;
                foreach (var item in order.Items)
                {
                    var product = products.FirstOrDefault(p => p.Id == item.ProductId);
                    if (product != null)
                    {
                        product.Quantity += item.Quantity;
                        Console.WriteLine($" + {item.Quantity} units returned to {product.Name} (New Stock: {product.Quantity})");
                    }
                    else
                    {
                        Console.WriteLine($" WARNING: Product ID {item.ProductId} ({item.ProductName}) not found in inventory. Stock not returned for this item.");
                        stockReturnError = true;
                    }
                }
                if (stockReturnError)
                {
                    Console.WriteLine("Warning: Some products for the cancelled order were not found. Inventory might be inconsistent.");
                }
            }
            // --- End Stock Adjustment ---

            string oldStatus = order.Status;
            order.Status = newStatus;

            // Optionally update paid status if moving to Completed
            if (newStatus.Equals("Completed", StringComparison.OrdinalIgnoreCase) && !order.IsPaid)
            {
                Console.Write("Mark order as Paid? (Y/N) [Y]: ");
                if (Console.ReadLine()?.Trim().ToUpper() != "N") // Default to Yes
                {
                    order.IsPaid = true;
                    Console.WriteLine("Order marked as Paid.");
                    currentUser.LogActivity("Order Status Update", $"Marked order {order.OrderId} as Paid.");
                }
            }

            currentUser.LogActivity("Order Status Update", $"Changed status for Order {order.OrderId} from {oldStatus} to {newStatus}.");
            Console.WriteLine($"Order {order.OrderId} status updated to {newStatus}.");
        }

        // --- Reporting ---
        public void ManageReports()
        {
            if (currentUser == null || (currentUser.Role != "Admin" && currentUser.Role != "Manager"))
            {
                Console.WriteLine("Access denied. Manager or Admin role required.");
                return;
            }

            bool managing = true;
            while (managing)
            {
                Console.WriteLine("\n===== REPORTS =====");
                Console.WriteLine("1. Inventory Report");
                Console.WriteLine("2. Inventory by Category Report");
                Console.WriteLine("3. Financial Report (Sales)");
                Console.WriteLine("4. Supplier Report");
                Console.WriteLine("0. Back to Main Menu");
                Console.Write("\nEnter your choice: ");

                if (int.TryParse(Console.ReadLine(), out int choice))
                {
                    switch (choice)
                    {
                        case 0:
                            managing = false;
                            break;
                        case 1:
                            Report.GenerateInventoryReport(products);
                            break;
                        case 2:
                            Report.GenerateCategoryReport(products);
                            break;
                        case 3:
                            GenerateFinancialReportInteractive();
                            break;
                        case 4:
                            Report.GenerateSupplierReport(suppliers, purchaseOrders);
                            break;
                        default:
                            Console.WriteLine("Invalid option.");
                            break;
                    }
                    // Pause after showing report
                    if (choice > 0 && choice <= 4)
                    {
                        Console.WriteLine("\nPress Enter to continue...");
                        Console.ReadLine();
                    }
                }
                else { Console.WriteLine("Invalid input."); }
            }
        }

        private void GenerateFinancialReportInteractive()
        {
            Console.WriteLine("\n--- Generate Financial Report ---");
            Console.Write("Enter Start Date (YYYY-MM-DD): ");
            string startInput = Console.ReadLine();
            if (!DateTime.TryParseExact(startInput, "yyyy-MM-dd", CultureInfo.InvariantCulture, DateTimeStyles.None, out DateTime startDate))
            {
                Console.WriteLine("Invalid start date format. Using beginning of time.");
                startDate = DateTime.MinValue;
            }

            Console.Write("Enter End Date (YYYY-MM-DD) [Default: Today]: ");
            string endInput = Console.ReadLine();
            if (!DateTime.TryParseExact(endInput, "yyyy-MM-dd", CultureInfo.InvariantCulture, DateTimeStyles.None, out DateTime endDate))
            {
                endDate = DateTime.Today; // Use today if input is invalid/blank
            }
            // Ensure end date includes the whole day
            endDate = endDate.AddDays(1).AddTicks(-1);

            if (startDate > endDate)
            {
                Console.WriteLine("Start date cannot be after end date.");
                return;
            }

            Report.GenerateFinancialReport(orders, startDate, endDate);
        }
    } // End of InventoryManager class

    // --- Program Entry Point ---
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("======================================");
            Console.WriteLine(" E-Commerce Inventory Manager");
            Console.WriteLine("======================================");

            InventoryManager manager = new InventoryManager();

            // Load data at startup
            manager.LoadData();

            // Attempt Login
            if (manager.Login())
            {
                // Show main menu if login is successful
                manager.ShowMainMenu();
            }
            // Whether login was successful or not, or after logout/exit choice:
            // Save data before exiting
            manager.SaveData();

            Console.WriteLine("\nExiting application. Press any key to close...");
            Console.ReadKey();
        }
    }
} // End of namespace
